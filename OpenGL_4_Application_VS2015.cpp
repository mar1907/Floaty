/*
GPS Project by Supuran Marius
Finished on 09/01/2018
*/

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <Windows.h>
#include <mmsystem.h>
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"
#include "SkyBox.hpp"

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
GLuint depthModelLoc;

glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

glm::vec3 ambientLight = glm::vec3(1.0f, 1.0f, 1.0f);
GLuint ambientLightLoc;
glm::vec3 diffuseLight = glm::vec3(1.0f, 1.00f, 1.0f);
GLuint diffuseLightLoc;
glm::vec3 specularLight = glm::vec3(1.0f, 1.0f, 1.0f);
GLuint specularLightLoc;

glm::vec3 posLightColor;
GLuint posLightColorLoc;
glm::vec3 posLightPos;
GLuint posLightPosLoc;

glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

GLint neg;
GLuint negLoc;

glm::vec3 viewPos;
GLuint viewPosLoc;

gps::Camera myCamera(glm::vec3(0.0f, 0.0f, 8.0f), glm::vec3(0.0f, 0.0f, -10.0f));
float cameraSpeed = 0.1f;

bool pressedKeys[1024];
float angle = 0.0f;
float doorAngle = 0.0f;
float doorAdd = 0.1f;

gps::Model3D houseModel;
gps::Shader houseShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::Model3D groundModel;

gps::Model3D doorModel;

gps::Model3D windowModel;

gps::Model3D farmModel;

gps::Model3D cottageModel;

gps::Model3D cottageModel2;

gps::Model3D buildingModel;

gps::Model3D candleModel;

gps::Model3D batModel;


GLuint farmVerticesVBO;
GLuint farmVerticesEBO;
GLuint farmObjectVAO;
GLint farmTexture;

GLuint grassVBO;
GLuint grassEBO;
GLuint grassVAO;
GLint grassTexture;

glm::vec3 batPos(5, 3, 2);
float batAngle;
bool edge;

//vertex position and UV coordinates
GLfloat grassVertexData[] = {
	-10.0f, 0.0f, -10.0f,    0.0f, 0.0f,
	-10.0f, 0.0f, 0.0f,      0.0f, 1.0f,
	-10.0f, 0.0f, 10.0f,     0.0f, 0.0f,
	0.0f,  0.0f, -10.0f,	 1.0f, 0.0f,
	0.0f,  0.0f,  0.0f,		 1.0f, 1.0f,
	0.0f, 0.0f,   10.0f,	 1.0f, 0.0f,
	10.0f,  0.0f, -10.0f,	 0.0f, 0.0f,
	10.0f,  0.0f,  0.0f,	 0.0f, 1.0f,
	10.0f, 0.0f,   10.0f,	 0.0f, 0.0f,
};

GLuint grassVertexIndices[] = {
	0,1,4,
	0,4,3,
	1,2,5,
	1,5,4,
	3,4,7,
	3,7,6,
	4,5,8,
	4,8,7
};

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

std::vector<const GLchar*> faces;

double lastTimeStampSound = glfwGetTime();
double currentTimeStampSound;

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initFaces() {
	faces.push_back("skybox/hills_rt.tga");
	faces.push_back("skybox/hills_lf.tga");
	faces.push_back("skybox/hills_up.tga");
	faces.push_back("skybox/grass.tga");
	faces.push_back("skybox/hills_bk.tga");
	faces.push_back("skybox/hills_ft.tga");
}

void loadGrassData() {
	glGenVertexArrays(1, &grassVAO);

	glBindVertexArray(grassVAO);

	glGenBuffers(1, &grassVBO);
	glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertexData), grassVertexData, GL_STATIC_DRAW);

	glGenBuffers(1, &grassEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grassEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(grassVertexIndices), grassVertexIndices, GL_STATIC_DRAW);

	//vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//vertex texture
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

GLuint ReadTextureFromFile(const char* file_name) {
	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(
			stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
		);
	}

	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_SRGB, //GL_SRGB,//GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(houseShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

double xpos1 = glWindowWidth / 2, ypos1 = glWindowHeight / 2;
float horizontalAngle = glm::pi<float>(), verticalAngle = 0.0f;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	float pitch = ypos1 - ypos;
	float yaw = -xpos1 + xpos;

	myCamera.rotate(pitch / (8 * 240) * glm::pi<float>(), yaw / (8 * 320) * glm::pi<float>());

	xpos1 = xpos;
	ypos1 = ypos;

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(houseShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void isAtEdge(gps::Camera myCamera) {
	glm::vec3 pos = myCamera.getPosition();
	if (abs(pos.x) == 9 || abs(pos.z) == 9) {
		edge = true;
	}
}

void isInHouse(gps::Camera myCamera) {
	glm::vec3 pos = myCamera.getPosition();
	if (abs(pos.x) <= 1 && abs(pos.z) <= 1 && abs(pos.y) <= 3) {
		edge = false;
	}
}

bool autoMove = false;
bool explode = false;
bool point = true;

void processMovement()
{

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 1.0f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 1.0f;
		if (angle < 0.0f)
			angle += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_R]) {
		doorAdd = 1.0f;
	}

	if (pressedKeys[GLFW_KEY_T]) {
		doorAdd = -1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_X]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_M]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_N]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_F]) {
		if(!autoMove) {
			myCamera= gps::Camera(glm::vec3(0.0f, 0.0f, 8.0f), glm::vec3(0.0f, 0.0f, -10.0f));
			autoMove = true;
		}
	}

	if (pressedKeys[GLFW_KEY_G]) {
		if (autoMove) {
			autoMove = false;
		}
	}

	if (pressedKeys[GLFW_KEY_O]) {
		neg = 1;
		houseShader.useShaderProgram();
		negLoc = glGetUniformLocation(houseShader.shaderProgram, "neg");
		glUniform1i(negLoc, neg);
		skyboxShader.useShaderProgram();
		negLoc = glGetUniformLocation(skyboxShader.shaderProgram, "neg");
		glUniform1i(negLoc, neg);
	}

	if (pressedKeys[GLFW_KEY_P]) {
		neg = 0;
		houseShader.useShaderProgram();
		negLoc = glGetUniformLocation(houseShader.shaderProgram, "neg");
		glUniform1i(negLoc, neg);
		skyboxShader.useShaderProgram();
		negLoc = glGetUniformLocation(skyboxShader.shaderProgram, "neg");
		glUniform1i(negLoc, neg);
	}

	if (pressedKeys[GLFW_KEY_Y]) {
		if (!explode) {
			explode = true;
		}
	}

	if (pressedKeys[GLFW_KEY_U]) {
		if (explode) {
			explode = false;
		}
	}

	if (pressedKeys[GLFW_KEY_V]) {
		if (!point) {
			point = true;
		}
	}

	if (pressedKeys[GLFW_KEY_B]) {
		if (point) {
			point = false;
		}
	}

	if (autoMove) {
		angle += 1.0f;
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels()
{
	houseModel = gps::Model3D("objects/house/OBJ/Warehouse.obj", "objects/house/");
	groundModel = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	doorModel = gps::Model3D("objects/door/door.obj", "objects/door/");
	windowModel = gps::Model3D("objects/window/window.obj", "objects/window/");
	farmModel = gps::Model3D("objects/farmhouse/Farmhouse.obj", "objects/farmhouse/");
	cottageModel = gps::Model3D("objects/cottage/abandoned_cottage.obj", "objects/cottage/");
	cottageModel2 = gps::Model3D("objects/cottage2/Snow covered CottageOBJ.obj", "objects/cottage2/");
	buildingModel = gps::Model3D("objects/building/saintriqT3DS.obj", "objects/building/");
	candleModel = gps::Model3D("objects/lamp/untitled.obj", "objects/lamp/");
	batModel = gps::Model3D("objects/bat/batred.obj", "objects/bat/");
}

void initShaders()
{
	houseShader.loadShader("shaders/shaderHouse.vert", "shaders/shaderHouse.geom", "shaders/shaderHouse.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

void initUniforms()
{
	depthMapShader.useShaderProgram();
	depthModelLoc = glGetUniformLocation(depthMapShader.shaderProgram, "model");
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(model));

	houseShader.useShaderProgram();

	lightDirMatrixLoc = glGetUniformLocation(houseShader.shaderProgram, "lightDirMatrix");

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(houseShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(houseShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(houseShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(houseShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(1.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(houseShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(houseShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	ambientLightLoc = glGetUniformLocation(houseShader.shaderProgram, "ambientLight");
	glUniform3fv(ambientLightLoc, 1, glm::value_ptr(ambientLight));

	diffuseLightLoc = glGetUniformLocation(houseShader.shaderProgram, "diffuseLight");
	glUniform3fv(diffuseLightLoc, 1, glm::value_ptr(diffuseLight));

	specularLightLoc = glGetUniformLocation(houseShader.shaderProgram, "specularLight");
	glUniform3fv(specularLightLoc, 1, glm::value_ptr(specularLight));

	posLightPos = glm::vec3(0.0f, 0.0f, 0.0f);
	posLightPosLoc = glGetUniformLocation(houseShader.shaderProgram, "posLightPos");
	glUniform3fv(posLightPosLoc, 1, glm::value_ptr(posLightPos));

	//set light color
	posLightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	posLightColorLoc = glGetUniformLocation(houseShader.shaderProgram, "posLightColor");
	glUniform3fv(posLightColorLoc, 1, glm::value_ptr(posLightColor));

	//set view pos
	viewPos = myCamera.getPosition();
	viewPosLoc = glGetUniformLocation(houseShader.shaderProgram, "viewPos");
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

	//negative colors
	neg = 0;
	houseShader.useShaderProgram();
	negLoc = glGetUniformLocation(houseShader.shaderProgram, "neg");
	glUniform1i(negLoc, neg);
	skyboxShader.useShaderProgram();
	negLoc = glGetUniformLocation(skyboxShader.shaderProgram, "neg");
	glUniform1i(negLoc, neg);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	float newAngle = angle + 45.0f;
	if (newAngle > 360.0f) newAngle -= 360.0f;
	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(newAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void renderScene()
{
	glm::mat4 scaledModel;
	glm::mat4 doorModelMat;
	glm::mat4 windowModelMat;
	glm::mat4 candleModelMat;
	glm::mat4 farmModelMat;
	glm::mat4 batModelMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.5, 0.5, 0.5, 1.0);
	isAtEdge(myCamera);
	isInHouse(myCamera);
	
	glm::vec3 batInitForward(0, 0, 1);

	processMovement();
	
	//1st pass

	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	

	//initialize the model matrix
	model = glm::mat4(1.0f);
	//draw house
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(0, -2, 0));
	scaledModel = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	houseModel.Draw(depthMapShader);

	//draw bat
	if (edge) {
		batPos += 0.01f*(myCamera.getPosition() - batPos);
		batAngle = glm::acos(glm::dot(batInitForward, myCamera.getPosition() - batPos) / (glm::length(batInitForward)*glm::length(myCamera.getPosition() - batPos)));
		if ((myCamera.getPosition() - batPos).x < 0) {
			batAngle = -batAngle + 2*glm::pi<float>();
		}
		batModelMat = glm::translate(model, batPos);
		batModelMat = glm::rotate(batModelMat, batAngle, glm::vec3(0, 1, 0));
		scaledModel = glm::scale(batModelMat, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
		batModel.Draw(depthMapShader);
	}

	//draw ground
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);


	
	//draw door 1
	doorModelMat = glm::translate(model, glm::vec3(-0.45f, 0.69f, 2.1f));
	doorModelMat = glm::rotate(doorModelMat, glm::radians(doorAngle), glm::vec3(0, 1, 0));
	doorModelMat = glm::translate(doorModelMat, glm::vec3(1.33f, 0, 0.92f));
	scaledModel = glm::scale(doorModelMat, glm::vec3(0.46f, 0.275f, 1));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	doorModel.Draw(depthMapShader);


	//draw door 2
	doorModelMat = glm::translate(model, glm::vec3(-0.45f, 0.69f, -2.1f));
	doorModelMat = glm::rotate(doorModelMat, glm::radians(-doorAngle), glm::vec3(0, 1, 0));
	doorModelMat = glm::translate(doorModelMat, glm::vec3(1.33f, 0, 0.92f));
	scaledModel = glm::scale(doorModelMat, glm::vec3(0.46f, 0.275f, 1));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	doorModel.Draw(depthMapShader);

	//draw window 1
	windowModelMat = glm::translate(model, glm::vec3(1.1f, 0.8f, 0.7f));
	scaledModel = glm::scale(windowModelMat, glm::vec3(0.2f, 0.19f, 0.55f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	windowModel.Draw(depthMapShader);

	//draw window 2
	windowModelMat = glm::translate(model, glm::vec3(1.1f, 0.8f, -0.7f));
	scaledModel = glm::scale(windowModelMat, glm::vec3(0.2f, 0.19f, 0.55f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	windowModel.Draw(depthMapShader);

	//draw farm 1
	farmModelMat = glm::translate(model, glm::vec3(7.5f, 0, -10.8f));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(90.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	farmModel.Draw(depthMapShader);

	//draw farm 2
	farmModelMat = glm::translate(model, glm::vec3(-8.1f, 0, -10.8f));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(0.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	farmModel.Draw(depthMapShader);


	//draw cottage 1
	farmModelMat = glm::translate(model, glm::vec3(-1.15, 0, -10.8f));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.0097f, 0.01f, 0.01f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	cottageModel.Draw(depthMapShader);

	//draw cottage 2
	farmModelMat = glm::translate(model, glm::vec3(-8.5f, 0, 0));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(90.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.0097f, 0.01f, 0.01f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	cottageModel.Draw(depthMapShader);

	//draw building
	farmModelMat = glm::translate(model, glm::vec3(-8, 0, 8));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(90.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.8f, 0.8f, 0.8f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	buildingModel.Draw(depthMapShader);

	//draw snow cottage 1
	farmModelMat = glm::translate(model, glm::vec3(0.2f, 0, 10.5f));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(270.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.08f, 0.08f, 0.08f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	cottageModel2.Draw(depthMapShader);

	//draw farm 3
	farmModelMat = glm::translate(model, glm::vec3(8.5f, 0, -2.8f));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(180.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	farmModel.Draw(depthMapShader);

	//draw candle
	candleModelMat = glm::translate(model, glm::vec3(0, 0.1f, 0));
	scaledModel = glm::scale(candleModelMat, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(depthModelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	candleModel.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);







	//show objects 2nd pass

	houseShader.useShaderProgram();

	glUniform1f(glGetUniformLocation(houseShader.shaderProgram, "time"), glfwGetTime());
	if (explode) {
		glUniform1i(glGetUniformLocation(houseShader.shaderProgram, "exp"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(houseShader.shaderProgram, "exp"), 0);
	}

	if (point) {
		glUniform1i(glGetUniformLocation(houseShader.shaderProgram, "point"), 1);
	}
	else {
		glUniform1i(glGetUniformLocation(houseShader.shaderProgram, "point"), 0);
	}

	glUniformMatrix4fv(glGetUniformLocation(houseShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//initialize the view matrix
	view = myCamera.getViewMatrix();
	//send view matrix data to shader	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(houseShader.shaderProgram, "shadowMap"), 3);

	//initialize the model matrix
	model = glm::mat4(1.0f);
	//draw house
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(0, -2, 0));
	scaledModel = glm::scale(model,glm::vec3(0.5, 0.5, 0.5));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	houseModel.Draw(houseShader);

	//draw bat
	if (edge) {
		batModelMat = glm::translate(model, batPos);
		batModelMat = glm::rotate(batModelMat, batAngle, glm::vec3(0, 1, 0));
		scaledModel = glm::scale(batModelMat, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
		normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		batModel.Draw(houseShader);
		if (glm::length(batPos - myCamera.getPosition()) < 1) {
			currentTimeStampSound = glfwGetTime();
			if (currentTimeStampSound - lastTimeStampSound > 2) {
				PlaySound(TEXT("sound/scream.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}
			lastTimeStampSound = glfwGetTime();
		}
	}


	//draw ground
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(houseShader.shaderProgram, "diffuseTexture"), 0);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glBindVertexArray(grassVAO);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);


	//draw door 1
	doorModelMat = glm::translate(model, glm::vec3(-0.45f, 0.69f, 2.1f));
	doorModelMat = glm::rotate(doorModelMat, glm::radians(doorAngle), glm::vec3(0, 1, 0));
	doorModelMat = glm::translate(doorModelMat, glm::vec3(1.33f, 0, 0.92f));
	scaledModel = glm::scale(doorModelMat, glm::vec3(0.46f, 0.275f, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	doorModel.Draw(houseShader);


	//draw door 2
	doorModelMat = glm::translate(model, glm::vec3(-0.45f, 0.69f, -2.1f));
	doorModelMat = glm::rotate(doorModelMat, glm::radians(-doorAngle), glm::vec3(0, 1, 0));
	doorModelMat = glm::translate(doorModelMat, glm::vec3(1.33f, 0, 0.92f));
	scaledModel = glm::scale(doorModelMat, glm::vec3(0.46f, 0.275f, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	doorModel.Draw(houseShader);

	//glm::mat4 windowModelMat;
	windowModelMat = glm::translate(model, glm::vec3(1.1f, 0.8f, 0.7f));
	scaledModel = glm::scale(windowModelMat, glm::vec3(0.2f, 0.19f, 0.55f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	windowModel.Draw(houseShader);

	//draw window 2
	windowModelMat = glm::translate(model, glm::vec3(1.1f, 0.8f, -0.7f));
	scaledModel = glm::scale(windowModelMat, glm::vec3(0.2f, 0.19f, 0.55f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	windowModel.Draw(houseShader);

	//glm::mat4 farmModelMat;
	farmModelMat = glm::translate(model, glm::vec3(7.5f, 0, -10.8f));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(90.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	farmModel.Draw(houseShader);

	//draw farm 2
	farmModelMat = glm::translate(model, glm::vec3(-8.1f, 0, -10.8f));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(0.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	farmModel.Draw(houseShader);
	

	//draw cottage 1
	farmModelMat = glm::translate(model, glm::vec3(-1.15, 0, -10.8f));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.0097f, 0.01f, 0.01f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	cottageModel.Draw(houseShader);

	//draw cottage 2
	farmModelMat = glm::translate(model, glm::vec3(-8.5f, 0, 0));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(90.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.0097f, 0.01f, 0.01f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	cottageModel.Draw(houseShader);

	//draw building
	farmModelMat = glm::translate(model, glm::vec3(-8, 0, 8));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(90.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.8f, 0.8f, 0.8f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	buildingModel.Draw(houseShader);

	//draw snow cottage 1
	farmModelMat = glm::translate(model, glm::vec3(0.2f, 0, 10.5f));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(270.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.08f, 0.08f, 0.08f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	cottageModel2.Draw(houseShader);

	//draw farm 3
	farmModelMat = glm::translate(model, glm::vec3(8.5f, 0, -2.8f));
	farmModelMat = glm::rotate(farmModelMat, glm::radians(180.0f), glm::vec3(0, 1, 0));
	scaledModel = glm::scale(farmModelMat, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	farmModel.Draw(houseShader);

	//draw candle
	candleModelMat = glm::translate(model, glm::vec3(0, 0.1f, 0));
	scaledModel = glm::scale(candleModelMat, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(scaledModel));
	normalMatrix = glm::mat3(glm::inverseTranspose(view*scaledModel));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	candleModel.Draw(houseShader);



	skyboxShader.useShaderProgram();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mySkyBox.GetTextureId());
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	mySkyBox.Draw(skyboxShader, view, projection);
}

void initSkyBox() {
	initFaces();
	mySkyBox.Load(faces);
}

float movementSpeed = 90;
double lastTimeStamp = glfwGetTime();

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initModels();
	initShaders();
	initUniforms();	
	loadGrassData();
	initSkyBox();
	initFBOs();
	grassTexture = ReadTextureFromFile("textures/GrassGreenTexture0003.jpg");

	double currentTimeStamp;
	double elapsedSeconds;

	while (!glfwWindowShouldClose(glWindow)) {
		// get current time
		currentTimeStamp = glfwGetTime();
		elapsedSeconds = currentTimeStamp - lastTimeStamp;
		lastTimeStamp = currentTimeStamp;
		if (doorAdd == -1 && doorAngle > -90) doorAngle += -movementSpeed * elapsedSeconds;
		if (doorAdd == 1 && doorAngle < 0) doorAngle += movementSpeed * elapsedSeconds;
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
