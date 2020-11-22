#pragma once

#include "Scene.h"
#include <SDL_mixer.h>

extern Mix_Chunk *jumpSound;

class Level : public Scene {
public:
    void ProcessInput() override;
};
