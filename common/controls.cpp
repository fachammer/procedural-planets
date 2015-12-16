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
		vRot += deltaTime * vRotateSpeed;
		if (vRot > 90)
			vRot = 90;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		vRot -= deltaTime * vRotateSpeed;
		if (vRot < -90)
			vRot = -90;
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
		if (dist < 10) dist = 10;
	}
	//Move away from planet
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		dist += speed * deltaTime;
		if (dist > 1000) dist = 1000;
	}

	double hRad = hRot * 3.1415 / 180;
	double vRad = vRot * 3.1415 / 180;

	/*double x = dist * sin(vRad) * cos(hRad);
	double y = dist * sin(vRad) * sin(hRad);
	double z = dist * cos(vRad);

	glm::vec4 wrongPos = glm::vec4(x, y, z, 0);
	
	float aaa[16] = {
		1, 0, 0, 0,
		0, 0, 1, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	};
	glm::mat4 bbb;
	memcpy((&bbb), aaa, sizeof(aaa));

	wrongPos = (bbb * wrongPos);
	position = glm::vec3(wrongPos.x, wrongPos.y, wrongPos.z);*/

	double x = dist * cos(vRad) * sin(hRad);
	double y = dist * sin(vRad);
	double z = dist * cos(vRad) * cos(hRad);

	position = glm::vec3(x, y, z);

	glm::vec3 direction = -position;

	printf("\n(%6.4lf, %6.4lf, %6.4lf)", position.x, position.y, position.z);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(initialFoV, 4.0f / 3.0f, 0.1f, 10000.0f);
	// Camera matrix	
	ViewMatrix = glm::lookAt(
		position,
		position+direction,
		up
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}