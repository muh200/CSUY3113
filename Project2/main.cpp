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

#include <cstdlib>
#include <ctime>

struct Rectangle {
	const glm::vec3 topLeft;
	const glm::vec3 bottomRight;
	glm::vec3 position;
};

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

float rightPaddleDirection = 0.0f;
Rectangle rightPaddle{glm::vec3(-0.25, 1, 0), glm::vec3(0.25, -1, 0), glm::vec3(4.75, 0, 0)};

float leftPaddleDirection = 0.0f;
Rectangle leftPaddle{glm::vec3(-0.25, 1, 0), glm::vec3(0.25, -1, 0), glm::vec3(-4.75, 0, 0)};

glm::vec3 ballDirection = glm::vec3(0, 0, 0);
Rectangle ball{glm::vec3(-0.25, 0.25, 0), glm::vec3(0.25, -0.25, 0), glm::vec3(0, 0, 0)};

float lastTicks = 0.0f;

const float VIEW_LEFT = -5.0f;
const float VIEW_RIGHT = 5.0f;
const float VIEW_BOTTOM = -3.75f;
const float VIEW_TOP = 3.75f;

bool randomBool() {
	return std::rand() < (RAND_MAX / 2);
}

void drawRectangle(const Rectangle& rect) {
	modelMatrix = glm::translate(glm::mat4(1.0f), rect.position);
	program.SetModelMatrix(modelMatrix);

	float vertices[] = {
		rect.topLeft[0],
		rect.bottomRight[1],
		rect.bottomRight[0],
		rect.bottomRight[1],
		rect.bottomRight[0],
		rect.topLeft[1],
		rect.topLeft[0],
		rect.bottomRight[1],
		rect.bottomRight[0],
		rect.topLeft[1],
		rect.topLeft[0],
		rect.topLeft[1],
	};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);
}

void movePaddle(Rectangle& paddle, float direction, float speed, float deltaTime) {
	paddle.position[1] += direction * speed * deltaTime;
	const float positionMax = VIEW_TOP - paddle.topLeft[1];
	const float positionMin = VIEW_BOTTOM - paddle.bottomRight[1];

	if (paddle.position[1] > positionMax) {
		paddle.position[1] = positionMax;
	} else if (paddle.position[1] < positionMin) {
		paddle.position[1] = positionMin;
	}
}

void moveBall(Rectangle& ball, glm::vec3 direction, float speed, float deltaTime) {
	ball.position += direction * speed * deltaTime;
	const float maxY = VIEW_TOP - ball.topLeft[1];
	const float minY = VIEW_BOTTOM - ball.bottomRight[1];
	const float minX = VIEW_LEFT - ball.topLeft[0];
	const float maxX = VIEW_RIGHT - ball.bottomRight[0];

	if (ball.position[1] < minY) {
		ball.position[1] = minY;
		ballDirection[1] *= -1;
	} else if (ball.position[1] > maxY) {
		ball.position[1] = maxY;
		ballDirection[1] *= -1;
	}
}

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Project 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);

	program.Load("shaders/vertex.glsl", "shaders/fragment.glsl");

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(VIEW_LEFT, VIEW_RIGHT, VIEW_BOTTOM, VIEW_TOP, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	glUseProgram(program.programID);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	std::srand(std::time(nullptr));
	ballDirection = glm::vec3(1, 1, 0);
	if (randomBool()) {
		ballDirection[0] = -1;
	}
	if (randomBool()) {
		ballDirection[1] = -1;
	}
}

void ProcessInput() {
	rightPaddleDirection = 0.0f;
	leftPaddleDirection = 0.0f;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			gameIsRunning = false;
		}
	}
	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	// The reason why I did it by adding the direction and not just
	// setting it is because I wanted the paddles to stop if the player
	// pressed both the up and down key at once.
	if (keys[SDL_SCANCODE_UP]) {
		rightPaddleDirection += 1.0f;
	}
	if (keys[SDL_SCANCODE_DOWN]) {
		rightPaddleDirection += -1.0f;
	}

	if (keys[SDL_SCANCODE_W]) {
		leftPaddleDirection += 1.0f;
	}
	if (keys[SDL_SCANCODE_S]) {
		leftPaddleDirection += -1.0f;
	}
}

void Update() {
	const float ticks = (float)SDL_GetTicks() / 1000.0f;
	const float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	const float paddleSpeed = 5.0f;

	movePaddle(rightPaddle, rightPaddleDirection, paddleSpeed, deltaTime);
	movePaddle(leftPaddle, leftPaddleDirection, paddleSpeed, deltaTime);

	const float ballSpeed = 5.0f;
	moveBall(ball, ballDirection, ballSpeed, deltaTime);
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);

	drawRectangle(rightPaddle);
	drawRectangle(leftPaddle);
	drawRectangle(ball);

	SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Initialize();

	while (gameIsRunning) {
		ProcessInput();
		Update();
		Render();
	}

	Shutdown();
	return 0;
}
