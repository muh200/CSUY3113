#pragma once

#include "Scene.h"
#include <SDL_mixer.h>

struct GameState {
    Map *map = nullptr;
    Entity *player = nullptr;
    Entity *enemies = nullptr;
    Entity *balls = nullptr;
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
