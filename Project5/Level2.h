#pragma once

#include "Scene.h"

extern int lives;

class Level2 : public Scene {
public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Render(ShaderProgram *program) override;
private:
    GLuint fontTextureID;
};
