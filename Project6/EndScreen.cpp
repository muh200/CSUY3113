#include "EndScreen.h"

EndScreen::EndScreen(const std::string& message) : message(message) {}

void EndScreen::Initialize() {
    fontTextureID = Util::LoadTexture("font1.png");
}

void EndScreen::ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
        }
    }
}

void EndScreen::Update(float deltaTime) {}

void EndScreen::Render(ShaderProgram *program) {
    Util::DrawText(program, fontTextureID, message, 0.5f, -0.25f, glm::vec3(0, 0, 0));
}
