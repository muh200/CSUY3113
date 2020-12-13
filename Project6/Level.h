#pragma once

#include "Scene.h"
#include <SDL_mixer.h>

extern Mix_Chunk *hitSound;

struct GameState {
    Map *map = nullptr;
    Entity *player = nullptr;
    Entity *enemies = nullptr;
    Entity *balls = nullptr;
    glm::vec3 *throwDirections = nullptr;
    float *throwPowers = nullptr;
};

class Level : public Scene {
public:
    virtual void Initialize();
    virtual void ProcessInput();
    virtual void Update(float deltaTime);
    virtual void Render(ShaderProgram *program);
private:
    GameState state;
    GLuint fontTextureID;
};
