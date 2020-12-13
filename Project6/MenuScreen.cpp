#include "MenuScreen.h"

void MenuScreen::Initialize() {
    fontTextureID = Util::LoadTexture("font1.png");
}

void MenuScreen::ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_RETURN:
                        nextScene = 0;
                        break;
                }
            break;
        }
    }
}

void MenuScreen::Update(float deltaTime) {}

void MenuScreen::Render(ShaderProgram *program) {
    Util::DrawText(program, fontTextureID, "School's Out: Press enter to start", 0.5f, -0.25f, glm::vec3(-4.0, 0, 0));
}
