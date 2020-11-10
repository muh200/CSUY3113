#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"
#include "Map.h"

#include <vector>
#include <cassert>
#include <algorithm>
#include <initializer_list>

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5
#define AI_COUNT 0

enum GameMode { PLAYING, WON, LOST };

struct GameState {
    Entity *player = nullptr;
    Map *map = nullptr;
    Entity *enemies = nullptr;
    GameMode mode = PLAYING;
    GLuint fontTextureID;
};

GameState state;

unsigned int level1_data[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,
};

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

void DrawText(ShaderProgram *program, GLuint fontTextureID, std::string text,
              float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for(size_t i = 0; i < text.size(); i++) {

        int index = (int)text[i];
        float offset = (size + spacing) * i;

        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;

        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
        });
        texCoords.insert(texCoords.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
        });
    } // end of for loop

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);

    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project 4", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    
    glClearColor(0.45f, 0.45f, 0.57f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
   
    // Initialize Game Objects
    
    // Initialize Player
    state.player = new Entity();
    state.player->position = glm::vec3(0, -2.25, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.8f, 0);
    state.player->speed = 2.0f;
    state.player->jumpPower = 8.0f;
    state.player->textureID = LoadTexture("player.png");
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

    // GLuint enemyTextureID = LoadTexture("enemy.png");

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

    GLuint mapTextureID = LoadTexture("tileset.png");
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 4, 1);

    state.fontTextureID = LoadTexture("font1.png");
}

void ProcessInput() {
    
    state.player->movement = glm::vec3(0);
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        state.player->jump = state.player->collidedBottom;
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->movement.x = -1.0f;
        state.player->animIndices = state.player->animLeft;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        state.player->movement.x = 1.0f;
        state.player->animIndices = state.player->animRight;
    }
    

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }

}

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }
    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player->Update(FIXED_TIMESTEP, state.player, nullptr, 0, state.map);
        for (int i = 0; i < AI_COUNT; ++i) {
            state.enemies[i].Update(FIXED_TIMESTEP, state.player, nullptr, 0, state.map);
        }

        for (int i = 0; i < AI_COUNT; ++i) {
            if (state.player->CheckCollision(&state.enemies[i])) {
                if (state.player->position.y > state.enemies[i].position.y &&
                    state.player->velocity.y < 0) {
                    state.enemies[i].isActive = false;
                } else {
                    state.mode = LOST;
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

        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;

    viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::translate(viewMatrix, glm::vec3(-state.player->position.x, 0, 0));
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    program.SetViewMatrix(viewMatrix);

    state.map->Render(&program);

    for (int i = 0; i < AI_COUNT; ++i) {
        state.enemies[i].Render(&program);
    }

    state.player->Render(&program);

    if (state.mode != PLAYING) {
        std::string message;
        if (state.mode == WON) {
            message = "You Win";
        } else {
            message = "You Lose";
        }
        DrawText(&program, state.fontTextureID, message, 0.5f, -0.25f, glm::vec3(-1.125, 0, 0));
    }

    SDL_GL_SwapWindow(displayWindow);
}


void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        if (state.mode == PLAYING) {
            Update();
            Render();
        }
    }

    Shutdown();
    return 0;
}
