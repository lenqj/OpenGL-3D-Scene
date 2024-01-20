#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <random>
// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// camera
gps::Camera myCamera(
	glm::vec3(25.0f, 3.0f, 25.0f),
	glm::vec3(5.0f, 5.0f, 10.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;


gps::Model3D tank;
gps::Model3D tankUpPart;
float tankAngle = 0.0f;
glm::vec3 tankPosition;

GLfloat angle;

// shaders
gps::Shader myBasicShader;

//view mode
int viewModelType = GL_FILL;

//zoom
float fov = 45.0f;


//fog
GLfloat fogDensity = 0.0f;
GLuint fogDensityLoc;


//2nd light
GLuint lampLightPosShader0;
GLuint lampLightColorShader0;
GLuint lampLightTargetShader0;
glm::vec3 lampLightPos0;
glm::vec3 lampLightColor;
glm::vec3 lampLightTarget0;

// 3rd light
GLuint lampLightPosShader1;
GLuint lampLightColorShader1;
GLuint lampLightTargetShader1;
glm::vec3 lampLightPos1;
glm::vec3 lampLightTarget1;
float lampLightSwitch = 1;
float globalLightSwitch = 1;

//mouse


float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch;
bool firstMouse;

//SKYBOX
gps::SkyBox mySkyBox;
gps::Shader shaderSkyBox;



// scene animation

gps::Camera presentationCamera(
	glm::vec3(25.0f, 3.0f, 0.0f),
	glm::vec3(-25.0f, -5.0f, -10.0f),
	glm::vec3(1.0f, 1.0f, 0.0f));
bool animation = 1;
const double animationDuration = 15.0f;
double startTime = 0.0f;

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.007f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
	projection = glm::perspective(glm::radians(fov),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 130.0f);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}


void initOpenGLWindow() {
	myWindow.Create(1920, 1080, "OpenGL Project Core");
}

void initDaySkyBox() {
	std::vector<const GLchar*> faces;
	faces.push_back("models/skybox/day/rt.tga");
	faces.push_back("models/skybox/day/lf.tga");
	faces.push_back("models/skybox/day/up.tga");
	faces.push_back("models/skybox/day/dn.tga");
	faces.push_back("models/skybox/day/bk.tga");
	faces.push_back("models/skybox/day/ft.tga");
	mySkyBox.Load(faces);
}

void initNightSkyBox() {
	std::vector<const GLchar*> faces;
	faces.push_back("models/skybox/night/rt.tga");
	faces.push_back("models/skybox/night/lf.tga");
	faces.push_back("models/skybox/night/up.tga");
	faces.push_back("models/skybox/night/dn.tga");
	faces.push_back("models/skybox/night/bk.tga");
	faces.push_back("models/skybox/night/ft.tga");
	mySkyBox.Load(faces);
}
void animationMovement() {
	if (animation) {
		double currentTimeStamp = glfwGetTime();
		double elapsedTime = currentTimeStamp - startTime;
		double t = std::min(1.0, elapsedTime / animationDuration);
		float rotationOffset = 70.0f;

		if (t < 0.2) {
			presentationCamera.move(gps::MOVE_LEFT, t * cameraSpeed);
			float rotationAngle = glm::radians(-rotationOffset) * t;
			presentationCamera.rotate(0.0f, -rotationAngle);
		}
		if (t < 0.6) {
			presentationCamera.move(gps::MOVE_BACKWARD, t * cameraSpeed);
			float rotationAngle = glm::radians(-rotationOffset) * t;
			presentationCamera.rotate(rotationAngle, 0.0f);
		}
		else {
			if (t < 0.8) {
				presentationCamera.move(gps::MOVE_RIGHT, t * cameraSpeed / 3.0f);
				float rotationAngle = glm::radians(-rotationOffset) * t;
				presentationCamera.rotate(rotationAngle, 0.0f);
			}
			else {
				float rotationAngle = glm::radians(-rotationOffset) * t * 3.0;
				presentationCamera.rotate(rotationAngle, 0.0f);
			}
		}

		view = presentationCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		if (t >= 1.0) {
			animation = false;
			presentationCamera = gps::Camera(
				glm::vec3(25.0f, 3.0f, 0.0f),
				glm::vec3(-25.0f, -5.0f, -10.0f),
				glm::vec3(1.0f, 1.0f, 0.0f));
		}
	}
}
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}

	if (pressedKeys[GLFW_KEY_UP]) {
		if (tankPosition.z + cameraSpeed * glm::vec3(0.0f, 0.0f, -1.0f).z > -5.0) {
			tankPosition += cameraSpeed * glm::vec3(0.0f, 0.0f, -1.0f);
		}
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		if (tankPosition.z + cameraSpeed * glm::vec3(0.0f, 0.0f, 1.0f).z < 5.0) {
			tankPosition += cameraSpeed * glm::vec3(0.0f, 0.0f, 1.0f);
		}
	}

	if (pressedKeys[GLFW_KEY_LEFT]) {
		tankAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_RIGHT]) {
		tankAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_F1] && action == GLFW_PRESS) {
		viewModelType = GL_FILL;
	}

	if (pressedKeys[GLFW_KEY_F2] && action == GLFW_PRESS) {
		viewModelType = GL_LINE;
	}

	if (pressedKeys[GLFW_KEY_F3] && action == GLFW_PRESS) {
		viewModelType = GL_POINT;
	}

	if (pressedKeys[GLFW_KEY_F4] && action == GLFW_PRESS) {
		fogDensity += 0.015f;
	}

	if (pressedKeys[GLFW_KEY_F5] && action == GLFW_PRESS) {
		fogDensity -= 0.015f;
	}
	if (pressedKeys[GLFW_KEY_F6] && action == GLFW_PRESS) {
		lampLightSwitch = 1.0f - lampLightSwitch;
		lampLightColor = glm::vec3(lampLightSwitch, lampLightSwitch, lampLightSwitch);
	}

	if (pressedKeys[GLFW_KEY_F7] && action == GLFW_PRESS) {
		globalLightSwitch = 1.0f - globalLightSwitch;
		if (globalLightSwitch == 1) {
			initDaySkyBox();
		}
		else {
			initNightSkyBox();
		}
		lightColor = glm::vec3(globalLightSwitch, globalLightSwitch, globalLightSwitch);
	}
	if (pressedKeys[GLFW_KEY_F8] && action == GLFW_PRESS) {
		startTime = glfwGetTime();
		animation = !animation;
	}

}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
	glfwSetScrollCallback(myWindow.getWindow(), scroll_callback);
	glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
	teapot.LoadModel("models/untitled.obj");
	tank.LoadModel("models/tank.obj");
	tankUpPart.LoadModel("models/tank2.obj");
}

void initShaders() {
	myBasicShader.loadShader(
		"shaders/basic.vert",
		"shaders/basic.frag");
	shaderSkyBox.loadShader("shaders/skybox.vert", "shaders/skybox.frag");
}

void initTank() {
	myBasicShader.useShaderProgram();
	tankPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
}
void renderTank() {
	myBasicShader.useShaderProgram();
	glm::mat4 tankModel = glm::translate(glm::mat4(1.0f), tankPosition);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tankModel));
	tank.Draw(myBasicShader);
}
void initTankUpPart() {
	myBasicShader.useShaderProgram();
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
}
void renderTankUpPart() {
	myBasicShader.useShaderProgram();
	glm::mat4 tankModel = glm::mat4(1.0f);
	tankModel = glm::translate(tankModel, tankPosition);
	glm::vec3 upperPartCenter = glm::vec3(-1.74561f, 2.554f, 16.0491f);
	tankModel = glm::translate(tankModel, upperPartCenter);
	tankModel = glm::rotate(tankModel, glm::radians(tankAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	tankModel = glm::translate(tankModel, -upperPartCenter);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tankModel));
	tankUpPart.Draw(myBasicShader);
}

void initUniforms() {

	myBasicShader.useShaderProgram();

	// create model matrix for teapot
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// compute normal matrix for teapot
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(fov),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 130.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(1.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	lightColor = glm::vec3(globalLightSwitch, globalLightSwitch, globalLightSwitch);
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");


	lampLightPos0 = glm::vec3(12.0f, 3.0f, -23.75f);
	lampLightPosShader0 = glGetUniformLocation(myBasicShader.shaderProgram, "lampLightPos0");
	glUniform3fv(lampLightPosShader0, 1, glm::value_ptr(lampLightPos0));

	lampLightColor = glm::vec3(lampLightSwitch, lampLightSwitch, lampLightSwitch);
	lampLightColorShader0 = glGetUniformLocation(myBasicShader.shaderProgram, "lampLightColor0");
	glUniform3fv(lampLightColorShader0, 1, glm::value_ptr(lampLightColor));

	lampLightTarget0 = glm::vec3(12.0f, 3.0f, -20.0f);
	lampLightTargetShader0 = glGetUniformLocation(myBasicShader.shaderProgram, "lampLightTarget0");
	glUniform3fv(lampLightTargetShader0, 1, glm::value_ptr(lampLightTarget0));


	lampLightPos1 = glm::vec3(-12.0f, 3.0f, -23.75f);
	lampLightPosShader1 = glGetUniformLocation(myBasicShader.shaderProgram, "lampLightPos1");
	glUniform3fv(lampLightPosShader1, 1, glm::value_ptr(lampLightPos1));

	lampLightColorShader1 = glGetUniformLocation(myBasicShader.shaderProgram, "lampLightColor1");
	glUniform3fv(lampLightColorShader1, 1, glm::value_ptr(lampLightColor));

	lampLightTarget1 = glm::vec3(-12.0f, 3.0f, -20.0f);
	lampLightTargetShader1 = glGetUniformLocation(myBasicShader.shaderProgram, "lampLightTarget1");
	glUniform3fv(lampLightTargetShader1, 1, glm::value_ptr(lampLightTarget1));


}

void renderTeapot(gps::Shader shader) {
	shader.useShaderProgram();
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	teapot.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//view mode
	glPolygonMode(GL_FRONT_AND_BACK, viewModelType);
	//2nd and 3rd lights 
	glUniform3fv(lampLightColorShader0, 1, glm::value_ptr(lampLightColor));
	glUniform3fv(lampLightColorShader1, 1, glm::value_ptr(lampLightColor));
	//global light
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	//fog
	glUniform1f(fogDensityLoc, fogDensity);


	mySkyBox.Draw(shaderSkyBox, view, projection);

	renderTeapot(myBasicShader);
}

void cleanup() {
	myWindow.Delete();
	//cleanup code for your own data
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_Q]) {
		angle -= 1.0f;
		// update model matrix for teapot
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		// update normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle += 1.0f;
		// update model matrix for teapot
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		// update normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
}

int main(int argc, const char* argv[]) {

	try {
		initOpenGLWindow();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
	initTank();
	initTankUpPart();
	initDaySkyBox();
	setWindowCallbacks();


	glCheckError();
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		processMovement();
		animationMovement();
		renderScene();
		renderTank();
		renderTankUpPart();
		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

	return EXIT_SUCCESS;
}
