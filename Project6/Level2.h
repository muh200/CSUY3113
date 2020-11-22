#pragma once

#include "Level.h"

extern int lives;

class Level2 : public Level {
public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Render(ShaderProgram *program) override;
private:
    GLuint fontTextureID;
};
