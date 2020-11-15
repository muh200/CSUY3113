#pragma once

#include "Scene.h"

class EndScreen : public Scene {
public:
    EndScreen(const std::string& message);
    void Initialize() override;
    void ProcessInput();
    void Update(float deltaTime) override;
    void Render(ShaderProgram *program) override;
private:
    std::string message;
    GLuint fontTextureID;
};
