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

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

float rightPaddleDirection = 0.0f;
float rightPaddlePosition = 0.0f;

float leftPaddleDirection = 0.0f;
float leftPaddlePosition = 0.0f;

glm::vec3 ballPosition = glm::vec3(0, 0, 0);
glm::vec3 ballDirection = glm::vec3(0, 0, 0);

float lastTicks = 0.0f;

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
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	glUseProgram(program.programID);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ballDirection = glm::vec3(0, -1.0f, 0);
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

	rightPaddlePosition += rightPaddleDirection * paddleSpeed * deltaTime;

	if (rightPaddlePosition > 2.75) {
		rightPaddlePosition = 2.75;
	} else if (rightPaddlePosition < -2.75) {
		rightPaddlePosition = -2.75;
	}

	leftPaddlePosition += leftPaddleDirection * paddleSpeed * deltaTime;

	if (leftPaddlePosition > 2.75) {
		leftPaddlePosition = 2.75;
	} else if (leftPaddlePosition < -2.75) {
		leftPaddlePosition = -2.75;
	}

	const float ballSpeed = 5.0f;
	ballPosition += ballDirection * ballSpeed * deltaTime;

	if (ballPosition[1] < -3.5) {
		ballPosition[1] = -3.5;
		ballDirection[1] *= -1;
	} else if (ballPosition[1] > 3.5) {
		ballPosition[1] = 3.5;
		ballDirection[1] *= -1;
	}
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(4.75f, rightPaddlePosition, 0.0f));

	program.SetModelMatrix(modelMatrix);

	float paddleVertices[] = { -0.25, -1.0, 0.25, -1.0, 0.25, 1.0, -0.25, -1.0, 0.25, 1.0, -0.25, 1.0 };

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, paddleVertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);


	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-4.75f, leftPaddlePosition, 0.0f));

	program.SetModelMatrix(modelMatrix);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, paddleVertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

	float ballVertices[] = { -0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25 };

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, ballPosition);

	program.SetModelMatrix(modelMatrix);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

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
