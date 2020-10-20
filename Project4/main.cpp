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

#include <vector>
#include <cassert>

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

#define PLATFORM_COUNT 29

enum GameMode { PLAYING, WON, LOST };

struct GameState {
    Entity *player;
    Entity *platforms;
    GameMode mode = PLAYING;
    GLuint fontTextureID;
};

GameState state;

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

void initializeEntities(Entity* start, Entity* end, glm::vec3 startPosition,
                     glm::vec3 step, EntityType type, GLuint textureID) {
    for (Entity* p = start; p != end; ++p) {
        p->textureID = textureID;
        p->position = startPosition;
        p->type = type;
        startPosition += step;
    }
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
    state.player->position = glm::vec3(0, 3.75, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -0.05, 0);
    state.player->speed = 1.0f;
    state.player->textureID = LoadTexture("rocket.png");
    state.player->width = 0.73f;
    state.player->type = PLAYER;

    state.platforms = new Entity[PLATFORM_COUNT];

    // For this code to work, the non-safe platforms should appear before the
    // landing platform.
    // Specifically, this is needed if we collide with both the landing platform
    // and a non-safe platform.
    // The Entity object only records the first collision so we must check if
    // the rocket hit a non-safe platform first.

    GLuint bombTextureID = LoadTexture("tnt_tile.png");

    Entity* currentPosition = state.platforms;
    int numberOfTiles;

    numberOfTiles = 6;
    initializeEntities(currentPosition,
                       currentPosition + numberOfTiles,
                       glm::vec3(-4.5, -3.25f, 0),
                       glm::vec3(1, 0, 0),
                       BOMB_TILE,
                       bombTextureID);
    currentPosition += numberOfTiles;

    numberOfTiles = 2;
    initializeEntities(currentPosition,
                       currentPosition + numberOfTiles,
                       glm::vec3(3.5, -3.25f, 0),
                       glm::vec3(1, 0, 0),
                       BOMB_TILE,
                       bombTextureID);
    currentPosition += numberOfTiles;

    numberOfTiles = 7;
    initializeEntities(currentPosition,
                       currentPosition + numberOfTiles,
                       glm::vec3(-4.5, -2.25f, 0),
                       glm::vec3(0, 1, 0),
                       BOMB_TILE,
                       bombTextureID);
    currentPosition += numberOfTiles;

    numberOfTiles = 7;
    initializeEntities(currentPosition,
                       currentPosition + numberOfTiles,
                       glm::vec3(4.5, -2.25f, 0),
                       glm::vec3(0, 1, 0),
                       BOMB_TILE,
                       bombTextureID);
    currentPosition += numberOfTiles;

    numberOfTiles = 4;
    initializeEntities(currentPosition,
                       currentPosition + numberOfTiles,
                       glm::vec3(-3.5, 1.25f, 0),
                       glm::vec3(1, 0, 0),
                       BOMB_TILE,
                       bombTextureID);
    currentPosition += numberOfTiles;

    numberOfTiles = 1;
    initializeEntities(currentPosition,
                       currentPosition + numberOfTiles,
                       glm::vec3(3.5, -0.25f, 0),
                       glm::vec3(-1, 0, 0),
                       BOMB_TILE,
                       bombTextureID);
    currentPosition += numberOfTiles;

    GLuint landingTileTextureID = LoadTexture("landing.png");

    numberOfTiles = 2;
    initializeEntities(currentPosition,
                       currentPosition + numberOfTiles,
                       glm::vec3(1.5, -3.25f, 0),
                       glm::vec3(1, 0, 0),
                       LANDING_TILE,
                       landingTileTextureID);
    currentPosition += numberOfTiles;

    assert(currentPosition - state.platforms == PLATFORM_COUNT);

    for (int i = 0; i < PLATFORM_COUNT; ++i) {
        state.platforms[i].Update(0, nullptr, 0);
    }

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
                    case SDLK_LEFT:
                        // Move the player left
                        break;
                        
                    case SDLK_RIGHT:
                        // Move the player right
                        break;
                        
                    case SDLK_SPACE:
                        // Some sort of action
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
        state.player->Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        bool hasCollided = state.player->collidedTop ||
            state.player->collidedBottom ||
            state.player->collidedRight ||
            state.player->collidedLeft;
        if (hasCollided) {
            if (state.player->collidedBottom &&
                state.player->collidedBottom->type == LANDING_TILE) {
                state.mode = WON;
            } else {
                state.mode = LOST;
            }
        }
        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < PLATFORM_COUNT; ++i) {
        state.platforms[i].Render(&program);
    }

    state.player->Render(&program);

    if (state.mode != PLAYING) {
        std::string message;
        if (state.mode == WON) {
            message = "Mission successful";
        } else {
            message = "Mission failed!!!!";
        }
        DrawText(&program, state.fontTextureID, message, 0.5f, -0.25f, glm::vec3(-2.25, 0, 0));
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
