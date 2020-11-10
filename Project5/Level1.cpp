#include "Level1.h"

#define AI_COUNT 0
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5

unsigned int level1_data[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,
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

    state.player->animRight = new int[state.player->animFrames];
    std::initializer_list<int> animRight = {4, 0, 1, 2, 3};
    assert(animRight.size() == (size_t)state.player->animFrames);
    std::copy(animRight.begin(), animRight.end(), state.player->animRight);

    state.player->animLeft = new int[state.player->animFrames];
    std::initializer_list<int> animLeft = {10, 6, 7, 8, 9};
    assert(animLeft.size() == (size_t)state.player->animFrames);
    std::copy(animLeft.begin(), animLeft.end(), state.player->animLeft);

    state.player->animUp = new int[state.player->animFrames];
    for (int i = 0; i < state.player->animFrames; i++) {
        state.player->animUp[i] = 5;
    }

    state.player->width = 0.5f;
    state.player->height = 0.8125f;
    state.player->type = PLAYER;
    state.player->animIndices = state.player->animRight;

    state.enemies = new Entity[AI_COUNT];

    // GLuint enemyTextureID = Util::LoadTexture("enemy.png");

    // for (int i = 0; i < AI_COUNT; ++i) {
    //     state.enemies[i] = Entity();
    //     state.enemies[i].type = ENEMY;
    //     state.enemies[i].speed = 0.25f;
    //     state.enemies[i].width = 0.60f;
    //     state.enemies[i].animCols = 4;
    //     state.enemies[i].animRows = 2;
    //     state.enemies[i].animTime = 0.20f;
    //     state.enemies[i].animFrames = 4;

    //     state.enemies[i].animRight = new int[state.enemies[i].animFrames];
    //     std::initializer_list<int> animRight = {3, 0, 1, 2};
    //     assert(animRight.size() == (size_t)state.enemies[i].animFrames);
    //     std::copy(animRight.begin(), animRight.end(), state.enemies[i].animRight);

    //     state.enemies[i].animLeft = new int[state.enemies[i].animFrames];
    //     std::initializer_list<int> animLeft = {7, 4, 5, 6};
    //     assert(animLeft.size() == (size_t)state.enemies[i].animFrames);
    //     std::copy(animLeft.begin(), animLeft.end(), state.enemies[i].animLeft);

    //     state.enemies[i].animIndices =  state.enemies[i].animLeft;
    //     state.enemies[i].textureID = enemyTextureID;
    //     state.enemies[i].acceleration = glm::vec3(0, -9.8f, 0);
    // }

    // state.enemies[0].aiType = WALKER;
    // state.enemies[0].aiState = WALKING;
    // state.enemies[0].position = glm::vec3(0, -3, 0);

    // state.enemies[1].aiType = JUMPER;
    // state.enemies[1].aiState = JUMPING;
    // state.enemies[1].jumpPower = 5;
    // state.enemies[1].position = glm::vec3(4, 0, 0);

    // state.enemies[2].aiType = PATROLLER;
    // state.enemies[2].aiState = PATROLLING;
    // state.enemies[2].position = glm::vec3(-1, 2, 0);
    // state.enemies[2].movement = glm::vec3(1, 0, 0);
    // state.enemies[2].animIndices = state.enemies[2].animRight;

    GLuint mapTextureID = Util::LoadTexture("tileset.png");
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 4, 1);
}

void Level1::Update(float deltaTime) {
    state.player->Update(deltaTime, state.player, nullptr, 0, state.map);
    for (int i = 0; i < AI_COUNT; ++i) {
        state.enemies[i].Update(deltaTime, state.player, nullptr, 0, state.map);
    }

    for (int i = 0; i < AI_COUNT; ++i) {
        if (state.player->CheckCollision(&state.enemies[i])) {
            if (state.player->position.y > state.enemies[i].position.y &&
                state.player->velocity.y < 0) {
                state.enemies[i].isActive = false;
            } else {
                return;
            }
        }
    }

    // bool won = true;
    // for (int i = 0; i < AI_COUNT; ++i) {
    //     if (state.enemies[i].isActive) won = false;
    // }

    // if (won) {
    //     state.mode = WON;
    //     return;
    // }
}

void Level1::Render(ShaderProgram *program) {
    state.map->Render(program);

    for (int i = 0; i < AI_COUNT; ++i) {
        state.enemies[i].Render(program);
    }

    state.player->Render(program);

    // if (state.mode != PLAYING) {
    //     std::string message;
    //     if (state.mode == WON) {
    //         message = "You Win";
    //     } else {
    //         message = "You Lose";
    //     }
    //     Util::DrawText(&program, state.fontTextureID, message, 0.5f, -0.25f, glm::vec3(-1.125, 0, 0));
    // }
}
