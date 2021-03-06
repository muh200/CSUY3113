#include "Level1.h"

#define LEVEL1_AI_COUNT 2
#define LEVEL1_WIDTH 17
#define LEVEL1_HEIGHT 5

unsigned int level1_data[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
};

void Level1::Initialize() {
    state.player = new Entity();
    state.player->position = glm::vec3(0, -2.25, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.8f, 0);
    state.player->speed = 2.0f;
    state.player->jumpPower = 8.0f;
    state.player->textureID = Util::LoadTexture("player.png");
    state.player->animCols = 6;
    state.player->animRows = 2;
    state.player->animTime = 0.20f;
    state.player->animFrames = 5;

    state.player->animRight = new int[5]{4, 0, 1, 2, 3};
    state.player->animLeft = new int[5]{10, 6, 7, 8, 9};

    state.player->width = 0.5f;
    state.player->height = 0.8125f;
    state.player->type = PLAYER;
    state.player->animIndices = state.player->animRight;

    state.enemies = new Entity[LEVEL1_AI_COUNT];

    GLuint enemyTextureID = Util::LoadTexture("enemy.png");

    for (int i = 0; i < LEVEL1_AI_COUNT; ++i) {
        state.enemies[i] = Entity();
        state.enemies[i].type = ENEMY;
        state.enemies[i].speed = 0.25f;
        state.enemies[i].width = 0.60f;
        state.enemies[i].animCols = 4;
        state.enemies[i].animRows = 2;
        state.enemies[i].animTime = 0.20f;
        state.enemies[i].animFrames = 4;

        state.enemies[i].animRight = new int[4]{3, 0, 1, 2};
        state.enemies[i].animLeft = new int[4]{7, 4, 5, 6};

        state.enemies[i].animIndices = state.enemies[i].animLeft;
        state.enemies[i].textureID = enemyTextureID;
        state.enemies[i].acceleration = glm::vec3(0, -9.8f, 0);

        state.enemies[i].aiType = PATROLLER;
        state.enemies[i].aiState = PATROLLING;
        state.enemies[i].movement = glm::vec3(1, 0, 0);
        state.enemies[i].animIndices = state.enemies[0].animRight;
    }


    GLuint mapTextureID = Util::LoadTexture("tileset.png");
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 2, 1);
    state.enemies[0].position = state.map->tileToCoord(7, 3);
    state.enemies[1].position = state.map->tileToCoord(12, 1);

    fontTextureID = Util::LoadTexture("font1.png");
}

void Level1::Update(float deltaTime) {
    state.player->Update(deltaTime, state.player, nullptr, 0, state.map);

    if (state.player->position.x >= state.map->tileToCoord(LEVEL1_WIDTH - 2, 0).x) {
        state.nextScene = 1;
        return;
    }

    for (int i = 0; i < LEVEL1_AI_COUNT; ++i) {
        state.enemies[i].Update(deltaTime, state.player, nullptr, 0, state.map);
    }

    bool lost = state.player->position.y < -3.75;
    for (int i = 0; i < LEVEL1_AI_COUNT && !lost; ++i) {
        if (state.player->CheckCollision(&state.enemies[i])) {
            if (state.player->position.y > state.enemies[i].position.y &&
                state.player->velocity.y < 0) {
                state.enemies[i].isActive = false;
            } else {
                lost = true;
            }
        }
    }

    if (lost) {
        --lives;
        if (lives == 0) {
            state.nextScene = 4;
            return;
        }
        state.player->position = glm::vec3(0, -2.25, 0);
    }
}

void Level1::Render(ShaderProgram *program) {
    state.map->Render(program);

    for (int i = 0; i < LEVEL1_AI_COUNT; ++i) {
        state.enemies[i].Render(program);
    }

    state.player->Render(program);

    Util::DrawText(program, fontTextureID, "Lives: " + std::to_string(lives), 0.5f, -0.25f, glm::vec3(0, 0, 0));
}
