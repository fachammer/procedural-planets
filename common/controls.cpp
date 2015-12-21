// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window;
extern bool wireFrameMode;
extern bool drawCoordinateMeshes;
extern bool setLightToCamera;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

glm::vec3 position;
// Initial Field of View
float initialFoV = 45.0f;

float defaultSpeed = 0.75f;
float speed = 300.f;
float defaultRotateSpeed = 0.003f;
float rotateSpeed = 1.5f;

glm::vec3 up = glm::vec3(0, 1, 0);
glm::vec3 targetPos = glm::vec3(0, 0, 0);

float rho = 500;
float theta = 0;
float phi = 0;

bool canChangeWireframeMode = true;
bool canChangeDrawCoordinateMeshes = true;

glm::vec3 getCameraPosition() {
    return position;
}

void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	
	//update move speed based on distance
	rotateSpeed = defaultRotateSpeed * rho;
	speed = defaultSpeed * rho;

	// change latitude
	if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS)
		phi += deltaTime * rotateSpeed;
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
		phi -= deltaTime * rotateSpeed;

	// change longitude
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
		theta -= deltaTime * rotateSpeed;
	if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS)
		theta += deltaTime * rotateSpeed;


	//Move towards and away from planet
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		rho -= speed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		rho += speed * deltaTime;

	//toggle wireframe mode
	int changeMode = glfwGetKey(window, GLFW_KEY_F);
	if (changeMode == GLFW_PRESS && canChangeWireframeMode) {
		wireFrameMode = !wireFrameMode;
		canChangeWireframeMode = false;
	}
	else if (changeMode == GLFW_RELEASE)
		canChangeWireframeMode = true;
    
    changeMode = glfwGetKey(window, GLFW_KEY_C);
    if (changeMode == GLFW_PRESS && canChangeDrawCoordinateMeshes) {
        drawCoordinateMeshes = !drawCoordinateMeshes;
        canChangeDrawCoordinateMeshes = false;
    }
    else if (changeMode == GLFW_RELEASE)
        canChangeDrawCoordinateMeshes = true;
    
    setLightToCamera = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

	//clamp distance and latitude
	phi = min(1.57f, max(-1.57f, phi));
	rho = min(1000.f, max(10.f, rho));
	
	position = glm::vec3(
		rho * cos(theta) * cos(phi), 
		rho * sin(phi), 
		rho * sin(theta) * cos(phi)
	);

	// Projection matrix
    int width, height;
    glfwGetWindowSize(window, &width, &height);
	ProjectionMatrix = glm::perspective(initialFoV, (float) width / height, 0.1f, 10000.0f);
	// Camera matrix	
	ViewMatrix = glm::lookAt(
		position,
		targetPos,
		up
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}