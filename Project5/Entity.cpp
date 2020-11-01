#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;
    
    modelMatrix = glm::mat4(1.0f);
}

void Entity::Update(float deltaTime, Entity* player, Entity *platforms, int platformCount)
{
    if (!isActive) return;

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (animIndices != NULL) {
        if (glm::length(movement) != 0) {
            animTime += deltaTime;

            if (animTime >= 0.25f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }
        } else {
            animIndex = 0;
        }
    }

    if (jump) {
        jump = false;
        velocity.y += jumpPower;
    }
    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;
    
    position.y += velocity.y * deltaTime;
    CheckCollisionsY(platforms, platformCount);

    position.x += velocity.x * deltaTime;
    CheckCollisionsX(platforms, platformCount);

    // The reason that this is here rather than the top of the method is
    // because of the jumping AI depends on collision flags which are reset
    // at the beginning of the method.
    if (type == ENEMY) {
        AI(player, platforms, platformCount);
    }

    position.x = glm::clamp(position.x, -5.0f + width/2, 5.0f - width/2);

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::AI(Entity* player, Entity *platforms, int platformCount) {
    switch (aiType) {
        case WALKER:
            AIWalker(player);
            break;
        case JUMPER:
            AIJumper(player);
            break;
        case PATROLLER:
            AIPatroller(platforms, platformCount);
            break;
    }
}


void Entity::AIWalker(Entity* player) {
    const float walkingThreshold = 0.05f;
    // The enemy should only change direction if the player is past a certian
    // distance from the player. Otherwise, we get flickering behavior when the
    // player is directly above the enemy.
    if (fabs(position.x - player->position.x) > walkingThreshold) {
        if (position.x < player->position.x) {
            movement.x = 1;
            animIndices = animRight;
        } else if (position.x > player->position.x) {
            movement.x = -1;
            animIndices = animLeft;
        }
    } else {
        movement.x = 0;
    }
}

void Entity::AIJumper(Entity* player) {
    const float jumpingThreshold = 0.05f;
    // The enemy should only change direction if the player is past a certian
    // distance from the player. Otherwise, we get flickering behavior when the
    // player is directly above the enemy.
    if (fabs(position.x - player->position.x) > jumpingThreshold) {
        if (position.x < player->position.x) {
            movement.x = 1;
            if (collidedBottom) jump = true;
            animIndices = animRight;
        } else if (position.x > player->position.x) {
            movement.x = -1;
            if (collidedBottom) jump = true;
            animIndices = animLeft;
        }
    } else {
        movement.x = 0;
    }
}

void Entity::AIPatroller(Entity *platforms, int platformCount) {
    glm::vec3 triggerPoint = position + glm::vec3(((movement.x > 0) ? 0.5 : -0.5), -1, 0);
    for (int i = 0; i < platformCount; i++) {
        const float leftX = platforms[i].position.x - platforms[i].width/2;
        const float rightX = platforms[i].position.x + platforms[i].width/2;
        const float upY = platforms[i].position.y + platforms[i].height/2;
        const float downY = platforms[i].position.y - platforms[i].height/2;
        if (triggerPoint.x < rightX &&
            triggerPoint.x > leftX &&
            triggerPoint.y < upY &&
            triggerPoint.y > downY) {
            return;
        }
    }
    movement.x *= -1;
    if (movement.x < 0) {
        animIndices = animLeft;
    } else {
        animIndices = animRight;
    }
}

void Entity::CheckCollisionsX(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity *object = &objects[i];
        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
            }
        }
    }
}

void Entity::CheckCollisionsY(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity *object = &objects[i];
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
            }
        }
    }
}

bool Entity::CheckCollision(Entity* other)
{
    if (!isActive || !other->isActive) return false;

    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    return xdist < 0 && ydist < 0;
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram *program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;
    
    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;
    
    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v};
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram *program) {
    if (!isActive) return;

    program->SetModelMatrix(modelMatrix);
    
    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
