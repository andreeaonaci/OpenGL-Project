#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include "SkyBox.hpp"

int glWindowWidth = 800;
int glWindowHeight = 600;
float var;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

// shadows
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

std::vector<const GLchar*> facesSkyBox;

int pointLight = 0;
int spotLight = 0;
glm::vec3 lightPos1;
GLuint lightPos1Loc;
glm::vec3 lightPos2;
GLuint lightPos2Loc;
float spotLightCutoff;
float spotLightInnerCutoff;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	//glm::vec3(0.0f, 197.56f, 241.05f),
	//glm::vec3(0.0f, 0.0f, 0.0f),
	//glm::vec3(0.0f, 20.733f, 0.0f)
	glm::vec3(0.0f, 2.0f, 200.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 20.0f, 0.0f)
);
float cameraSpeed = 0.5f;

bool pressedKeys[1024];
bool rotation;
float angleY = 0.0f;
float anglePigeon = 0.0f;
GLfloat lightAngle;

gps::Model3D island;
gps::Model3D sun;
gps::Model3D pigeon;
gps::Model3D pigeonWings;
gps::Model3D chest;
gps::Model3D screenQuad;
gps::Model3D swing;
gps::Model3D swingBars;
gps::Model3D umbrella;
gps::Model3D palm;
gps::Model3D leaves;
gps::Model3D boat;

gps::Shader myCustomShader;
gps::Shader depthMapShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader skyBoxShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

double startTime = glfwGetTime();
bool autoRotateBool = false;
float previewAngle = 0.6f;

int fogInitial = 0;
GLint fogInitLoc;
GLfloat fogDensity = 0.005f;
gps::SkyBox skyBox;

bool showDepthMap;

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	glViewport(0, 0, width, height);
	//TODO	
}

void previewFunction() {
	if (autoRotateBool) {
		previewAngle += 0.6f;
		myCamera.scenePreview(previewAngle, cameraSpeed);
	}
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}

	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		previewFunction();
		autoRotateBool = !autoRotateBool;
	}
}

float mouseSpeed = 0.0001f;
float horizontalAngle, verticalAngle;
int latime, inaltime;
float yaw = -90.0f, pitch = 0;
bool firstMouse = true;
float lastX = 500, lastY = 375;
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

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	myCamera.rotate(pitch, yaw);
}

float angleUmbrella = 0.0f;
void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		anglePigeon -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_E]) {
		anglePigeon += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
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

	//if (pressedKeys[GLFW_KEY_P]) {
	//	autoRotateBool = !autoRotateBool;
	//}

	//start point light
	if (pressedKeys[GLFW_KEY_R]) {
		myCustomShader.useShaderProgram();
		pointLight = 1;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight"), pointLight);
	}

	//stop point light
	if (pressedKeys[GLFW_KEY_T]) {
		myCustomShader.useShaderProgram();
		pointLight = 0;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointLight"), pointLight);
	}

	// start spotlight
	if (pressedKeys[GLFW_KEY_Y]) {
		myCustomShader.useShaderProgram();
		spotLight = 1;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight"), spotLight);
	}

	// stop spotlight
	if (pressedKeys[GLFW_KEY_U]) {
		myCustomShader.useShaderProgram();
		spotLight = 0;
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "spotLight"), spotLight);
	}

	// line view
	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// point view
	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	// normal view
	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// start fog
	if (pressedKeys[GLFW_KEY_B]) {

		myCustomShader.useShaderProgram();
		fogInitial = 1;
		fogInitLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogInitial");
		glUniform1i(fogInitLoc, fogInitial);

	}

	// stop fog
	if (pressedKeys[GLFW_KEY_V]) {
		myCustomShader.useShaderProgram();
		fogInitial = 0;
		fogInitLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogInitial");
		glUniform1i(fogInitLoc, fogInitial);

	}

	// increase the intensity of fog
	if (pressedKeys[GLFW_KEY_4])
	{
		fogDensity = glm::min(fogDensity + 0.0001f, 1.0f);
	}

	// decrease the intensity of fog
	if (pressedKeys[GLFW_KEY_5])
	{
		fogDensity = glm::max(fogDensity - 0.0001f, 0.0f);
	}

	if (pressedKeys[GLFW_KEY_I]) {
		angleUmbrella += 1.0f;
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	island.LoadModel("objects/island/Small_Tropical_Island1.obj");
	sun.LoadModel("objects/sun/sun.obj");
	pigeon.LoadModel("objects/pigeon/body.obj");
	swing.LoadModel("objects/swing/woodswing.obj");
	swing.LoadModel("objects/swing/swingBars.obj");
	umbrella.LoadModel("objects/umbrella/umbrela1.obj");
	boat.LoadModel("objects/ship/boat.obj");
	//palm.LoadModel("objects/palm/free_palm.obj");
	//leaves.LoadModel("objects/palm/coroane.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/lightSpaceTrMatrix.vert", "shaders/lightSpaceTrMatrix.frag");
	depthMapShader.useShaderProgram();
	skyBoxShader.loadShader("shaders/skyBoxShader.vert", "shaders/skyBoxShader.frag");
	skyBoxShader.useShaderProgram();
}

void skyBoxShaderInit()
{
	skyBox.Load(facesSkyBox);
	skyBoxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyBoxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
}

void facesForSkyBox()
{
	facesSkyBox.push_back("skyBox/right.tga");
	facesSkyBox.push_back("skyBox/left.tga");
	facesSkyBox.push_back("skyBox/top.tga");
	facesSkyBox.push_back("skyBox/bottom.tga");
	facesSkyBox.push_back("skyBox/back.tga");
	facesSkyBox.push_back("skyBox/front.tga");
}

float spotlight;

glm::vec3 spotLightDirection;
glm::vec3 spotLightPosition;

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 150.56f, 201.05f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	// pointlight
	lightPos1 = glm::vec3(100.0f, 100.56f, 101.05f);
	lightPos1Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos1");
	glUniform3fv(lightPos1Loc, 1, glm::value_ptr(lightPos1));

	//spotlight
	spotLightCutoff = glm::cos(glm::radians(10.0f));
	spotLightInnerCutoff = glm::cos(glm::radians(20.0f));

	spotLightDirection = glm::normalize(glm::vec3(0, -1, 0));
	spotLightPosition = glm::vec3(-65.402f, 2.73f, 103.23f);

	spotLightCutoff = 10.0f;
	spotLightInnerCutoff = 30.0f;
	
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLightCutoff"), spotLightCutoff);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "spotLightInnerCutoff"), spotLightInnerCutoff);

	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLightDirection"), 1, glm::value_ptr(spotLightDirection));
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));
	glCheckError();
	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	float lightAngleRadian = glm::radians(lightAngle);

	lightRotation = glm::rotate(glm::mat4(1.0f), lightAngleRadian, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 lightDirAux = glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir;

	glm::mat4 lightView = glm::lookAt(lightDirAux, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	const GLfloat near_plane = 0.8f, far_plane = 60.0f;
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);

	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	island.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 150.56f, 201.05f));
	model = glm::scale(model, glm::vec3(0.5f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	sun.Draw(shader);

	model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	model = glm::translate(model, glm::vec3(62.302f, -0.55492f, 100.27f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	swingBars.Draw(shader);

	float radius = 10.0f;
	float angularSpeed = 0.1f;
	float deltaTime = 0.01f;
	float angleWind = 0.0f;

	glm::vec3 centerOfCircle(100.0f, 103.56f, 104.05f);
	glm::vec3 initialPigeonPosition = centerOfCircle + glm::vec3(radius, 0.0f, 0.0f);

	model = glm::translate(glm::mat4(1.0f), initialPigeonPosition);
	model = glm::scale(model, glm::vec3(0.5f));

	anglePigeon -= angularSpeed * deltaTime;
	model = glm::rotate(model, anglePigeon, glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	pigeon.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(45.73f, -0.55492f, 60.27f));
	model = glm::scale(model, glm::vec3(0.5f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix)); 
	}

	swing.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-100.65f, 4.6584f, -228.71f));
	model = glm::scale(model, glm::vec3(0.5f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	boat.Draw(shader);

	radius = 0.0f;
	angularSpeed = 0.1f;
	deltaTime = 0.00001f;

	glm::vec3 centerOfCircleUmbrella(-65.547f, -5.88f, 105.59f);

	model = glm::translate(glm::mat4(1.0f), centerOfCircleUmbrella);
	model = glm::scale(model, glm::vec3(1.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	model = glm::rotate(model, glm::radians(angleUmbrella), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	umbrella.Draw(shader);

	skyBox.Draw(skyBoxShader, view, projection);
}

void renderScene() {
	// Compute the light space transformation matrix
    glm::mat4 lightSpaceTrMatrix = computeLightSpaceTrMatrix();

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	depthMapShader.useShaderProgram();
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 150.56f, 201.05f));
	model = glm::scale(model, glm::vec3(0.5f));

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	sun.Draw(depthMapShader);

	depthMapShader.useShaderProgram();
	model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	model = glm::translate(model, glm::vec3(62.302f, -0.55492f, 100.27f));

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	swingBars.Draw(depthMapShader);

	depthMapShader.useShaderProgram();
	float radius = 10.0f;
	float angularSpeed = 0.1f;
	float deltaTime = 0.01f;
	float angleWind = 0.0f;

	glm::vec3 centerOfCircle(100.0f, 103.56f, 104.05f);
	glm::vec3 initialPigeonPosition = centerOfCircle + glm::vec3(radius, 0.0f, 0.0f);

	model = glm::translate(glm::mat4(1.0f), initialPigeonPosition);
	model = glm::scale(model, glm::vec3(0.5f));

	anglePigeon -= angularSpeed * deltaTime;
	model = glm::rotate(model, anglePigeon, glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	pigeon.Draw(depthMapShader);

	depthMapShader.useShaderProgram();
	model = glm::translate(glm::mat4(1.0f), glm::vec3(45.73f, -0.55492f, 60.27f));
	model = glm::scale(model, glm::vec3(0.5f));

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	swing.Draw(depthMapShader);

	depthMapShader.useShaderProgram();
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-100.65f, 4.6584f, -228.71f));
	model = glm::scale(model, glm::vec3(0.5f));

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	boat.Draw(depthMapShader);

	depthMapShader.useShaderProgram();
	radius = 0.0f;
	angularSpeed = 0.1f;
	deltaTime = 0.00001f;

	glm::vec3 centerOfCircleUmbrella(-65.402f, -2.73f, 103.23f);

	model = glm::translate(glm::mat4(1.0f), centerOfCircleUmbrella);
	model = glm::scale(model, glm::vec3(1.0f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	model = glm::rotate(model, glm::radians(angleUmbrella), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	umbrella.Draw(depthMapShader);

	depthMapShader.useShaderProgram();
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	island.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);
		glCheckError();
		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 42);

		glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);

		drawObjects(myCustomShader, false);

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 150.56f, 201.05f));
		model = glm::scale(model, glm::vec3(0.5f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		sun.Draw(lightShader);
	}
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();

	facesForSkyBox();
	skyBoxShaderInit();

	glCheckError();

	//glfwSetCursorPosCallback(glWindow, mouseCallback);

	//glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		float deltaTime = 0.05f;
		processMovement();
		previewFunction();
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
		glCheckError();
	}
	
	cleanup();
	
	return 0;
}
