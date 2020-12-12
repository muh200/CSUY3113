#include "Level.h"

#include <algorithm>

#define LEVEL1_AI_COUNT 2
#define LEVEL1_BALL_COUNT 3
#define LEVEL1_WIDTH 17
#define LEVEL1_HEIGHT 10

unsigned int level1_data[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void Level::Initialize() {
    state.player = new Entity();
    state.player->position = glm::vec3(0, -2.25, 0);
    state.player->movement = glm::vec3(0);
    state.player->speed = 2.0f;
    state.player->textureID = Util::LoadTexture("player.png");
    state.player->animCols = 3;
    state.player->animRows = 4;
    state.player->animTime = 0.20f;
    state.player->animFrames = 3;

    state.player->animDown = new int[3]{0, 1, 2};
    state.player->animLeft = new int[3]{3, 4, 5};
    state.player->animRight = new int[3]{6, 7, 8};
    state.player->animUp = new int[3]{9, 10, 11};

    // state.player->width = 0.5f;
    // state.player->height = 0.8125f;
    state.player->type = PLAYER;
    state.player->animIndices = state.player->animRight;

    state.enemies = new Entity[LEVEL1_AI_COUNT];

    GLuint enemyTextureID = Util::LoadTexture("enemy.png");

    for (int i = 0; i < LEVEL1_AI_COUNT; ++i) {
        state.enemies[i] = Entity();
        state.enemies[i].type = ENEMY;
        state.enemies[i].speed = 0.25f;
        state.enemies[i].width = 0.60f;
        state.enemies[i].animCols = 3;
        state.enemies[i].animRows = 4;
        state.enemies[i].animTime = 0.20f;
        state.enemies[i].animFrames = 3;

        state.enemies[i].animDown = new int[3]{0, 1, 2};
        state.enemies[i].animLeft = new int[3]{3, 4, 5};
        state.enemies[i].animRight = new int[3]{6, 7, 8};
        state.enemies[i].animUp = new int[3]{9, 10, 11};

        state.enemies[i].animIndices = state.enemies[i].animLeft;
        state.enemies[i].textureID = enemyTextureID;

        state.enemies[i].movement = glm::vec3(1, 0, 0);
        state.enemies[i].animIndices = state.enemies[0].animRight;
    }

    GLuint ballTextureID = Util::LoadTexture("ball.png");
    state.balls = new Entity[LEVEL1_BALL_COUNT];
    for (int i = 0; i < LEVEL1_BALL_COUNT; ++i) {
        state.balls[i] = Entity();
        state.balls[i].type = BALL;
        state.balls[i].textureID = ballTextureID;

        state.balls[i].width = 0.5f;
        state.balls[i].height = 0.5f;
        state.balls[i].scale = 0.5f;

        state.balls[i].position = glm::vec3(0, 0, 0);
    }

    GLuint mapTextureID = Util::LoadTexture("tileset.png");
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 2, 1);
    state.enemies[0].position = state.map->tileToCoord(7, 3);
    state.enemies[1].position = state.map->tileToCoord(12, 1);

    fontTextureID = Util::LoadTexture("font1.png");
}

void Level::ProcessInput() {
    state.player->movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {}
                break; // SDL_KEYDOWN
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->movement.x = -1.0f;
        state.player->animIndices = state.player->animLeft;
    } else if (keys[SDL_SCANCODE_RIGHT]) {
        state.player->movement.x = 1.0f;
        state.player->animIndices = state.player->animRight;
    } else if (keys[SDL_SCANCODE_UP]) {
        state.player->movement.y = 1.0f;
        state.player->animIndices = state.player->animUp;
    } else if (keys[SDL_SCANCODE_DOWN]) {
        state.player->movement.y = -1.0f;
        state.player->animIndices = state.player->animDown;
    }

    int x, y;
    if (SDL_GetMouseState(&x, &y)) {
        float unit_x = ((x / 640.0f) * 10.0f) - 5.0f;
        float unit_y = (((480.0f - y) / 480.0f) * 7.5f) - 3.75;
        float xView = -std::clamp(state.player->position.x, 4.5f, LEVEL1_WIDTH - 5.5f);
        float yView = -std::clamp(state.player->position.y, -(LEVEL1_HEIGHT - 4.25f), -3.25f);
        unit_x -= xView;
        unit_y -= yView;
        for (int i = 0; i < LEVEL1_BALL_COUNT; ++i) {
            if (state.player->CheckCollision(&state.balls[i])) {
                state.player->movement = glm::vec3(0);
                state.balls[i].position = glm::vec3(unit_x, unit_y, 0);
                break;
            }
        }
    }

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void Level::Update(float deltaTime) {
    state.player->Update(deltaTime, state.player, nullptr, 0, state.map);
    state.player->position.x = std::clamp(state.player->position.x, 0.0f, LEVEL1_WIDTH - 1.0f);
    state.player->position.y = std::clamp(state.player->position.y, -(LEVEL1_HEIGHT - 1.0f), 0.0f);

    // if (state.player->position.x >= state.map->tileToCoord(LEVEL1_WIDTH - 2, 0).x) {
    //     nextScene = 1;
    //     return;
    // }

    // for (int i = 0; i < LEVEL1_AI_COUNT; ++i) {
    //     state.enemies[i].Update(deltaTime, state.player, nullptr, 0, state.map);
    // }

    for (int i = 0; i < LEVEL1_BALL_COUNT; ++i) {
        state.balls[i].Update(deltaTime, state.player, nullptr, 0, state.map);
    }

    for (int i = 0; i < LEVEL1_BALL_COUNT; ++i) {
        if (state.player->CheckCollision(&state.balls[i]) && glm::length(state.balls[i].velocity) == 0) {
            state.balls[i].position = state.player->position + glm::vec3(0, -0.5, 0);
            break;
        }
    }

    viewMatrix = glm::mat4(1.0f);
    float xView = -std::clamp(state.player->position.x, 4.5f, LEVEL1_WIDTH - 5.5f);
    float yView = -std::clamp(state.player->position.y, -(LEVEL1_HEIGHT - 4.25f), -3.25f);
    viewMatrix = glm::translate(viewMatrix, glm::vec3(xView, yView, 0.0f));
}

void Level::Render(ShaderProgram *program) {
    state.map->Render(program);

    // for (int i = 0; i < LEVEL1_AI_COUNT; ++i) {
    //     state.enemies[i].Render(program);
    // }

    state.player->Render(program);

    for (int i = 0; i < LEVEL1_BALL_COUNT; ++i) {
        state.balls[i].Render(program);
    }
}
