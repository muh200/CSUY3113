#pragma once

#include "Scene.h"

class MenuScreen : public Scene {
    void Initialize() override;
    void ProcessInput();
    void Update(float deltaTime) override;
    void Render(ShaderProgram *program) override;
private:
    GLuint fontTextureID;
};
