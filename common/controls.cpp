// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

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


// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, -500 );
// Initial horizontal angle : toward -Z
float horizontalAngle = 0;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 300.f; // 3 units / second
float hRotateSpeed = 150.f;
float vRotateSpeed = 150.f;
float mouseSpeed = 0.005f;

glm::vec3 up = glm::vec3(0, 1, 0);

float dist = 500;
float hRot = 0;
float vRot = 0;

void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	static double lastX =  xpos;
	static double lastY = ypos;

	// Reset mouse position for next frame
	//glfwSetCursorPos(window, 1024/2, 768/2);
	double dx = 0.0;
	double dy = 0.0;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		dx = lastX - xpos ;
		dy = lastY - ypos ;
	}

	lastX = xpos;
	lastY = ypos;

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(dx);
	verticalAngle   += mouseSpeed * float(dy );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
		vRot -= deltaTime * vRotateSpeed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		vRot += deltaTime * vRotateSpeed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
		hRot += deltaTime * hRotateSpeed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
		hRot -= deltaTime * hRotateSpeed;
	}
	//Move towards planet
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		dist -= speed * deltaTime;
		if (dist < 10) dist = 20;
	}
	//Move away from planet
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		dist += speed * deltaTime;
		if (dist > 1000) dist = 1000;
	}

	double hRad = hRot * 3.1415 / 180;
	double vRad = vRot * 3.1415 / 180;

	double x = dist * sin(vRad) * cos(hRad);
	double y = dist * sin(vRad) * sin(hRad);
	double z = dist * cos(vRad);

	double buffer = y;
	y = -z;
	z = buffer;

	position = glm::vec3(x, y, z);

	float FoV = initialFoV;

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(-x, -y, -z);

	// Right vector
	glm::vec3 right = cross(direction, glm::vec3(0, 1, 0));
	
	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 1.0f, 10000.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		glm::vec3(0, 0, 0), // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}