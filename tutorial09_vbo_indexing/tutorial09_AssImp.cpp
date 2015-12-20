// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

// Include GLEW
#include <GL/glew.h>

//#define MINGW_COMPILER

#ifdef __APPLE__
    #include "TargetConditionals.h"
    #ifdef TARGET_OS_MAC
        #define RESOLUTION_SCALE 2
        #define SCREENWIDTH 1250
        #define SCREENHEIGHT 1000
    #endif
#else
    #define RESOLUTION_SCALE 1
    #define SCREENWIDTH 1280
    #define SCREENHEIGHT 1024
#endif

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

bool wireFrameMode = false;
bool setLightToCamera = true;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/Mesh.hpp>
#include <common/GLError.h>
#include <common/RenderState.hpp>

// Include AntTweakBar
#include <AntTweakBar.h>

#include "SphereGenerator.hpp"

#define PATHTOCONTENT "../tutorial09_vbo_indexing/"

// Create and compile our GLSL program from the shaders
	std::string contentPath = PATHTOCONTENT;

const char *faceFile[6] = {
	"cm_left.bmp",
	"cm_right.bmp",
	"cm_top.bmp",
	"cm_bottom.bmp",
	"cm_back.bmp",
	"cm_front.bmp"
};

float shadowMagicNumber = 0.003;
unsigned char textureToShow = 0;
unsigned char layerToShow = 0;


struct Scene
{
	std::vector<RenderState*>* objects;
	std::vector<Mesh*>* meshes;
	std::vector<ShaderEffect*>* effects;

	Scene(std::vector<RenderState*>* _obj,
		std::vector<Mesh*>* _meshes,
		std::vector<ShaderEffect*>* _effects) :
			objects(_obj),
			meshes(_meshes),
			effects(_effects)
	{}
};

std::vector<unsigned int> coordinateMeshIndices;
bool drawCoordinateMeshes = false;
void renderObjects(Scene& scene, glm::mat4x4& viewMatrix, glm::mat4x4& projectionMatrix, glm::vec3& lightPos, glm::mat4& lightMatrix)
{
	std::vector<RenderState*>* objects = scene.objects;
	#ifdef MINGW_COMPILER
	glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	#else
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	#endif
	
	glPolygonMode(GL_FRONT_AND_BACK, (wireFrameMode ? GL_LINE : GL_FILL));

    check_gl_error();
	for(int i = 0; i < objects->size(); i++)
	{
        if(!drawCoordinateMeshes && std::find(coordinateMeshIndices.begin(), coordinateMeshIndices.end(), i) != coordinateMeshIndices.end())
            continue;
        
		RenderState* rs = (*objects)[i];
		
		unsigned int meshId = (*objects)[i]->meshId;
		Mesh* m = (*scene.meshes)[meshId];
		modelMatrix = m->modelMatrix;
		glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
		// Use our shader
		unsigned int effectId = (*objects)[i]->shaderEffectId;
		ShaderEffect* effect = (*scene.effects)[effectId];
		glUseProgram(effect->programId);
        check_gl_error();
		rs->setParameters(effect);
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(effect->MVPId, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(effect->MId, 1, GL_FALSE, &modelMatrix[0][0]);
		glUniformMatrix4fv(effect->VId, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniform1f(glGetUniformLocation(effect->programId, "maxNegativeHeight"), 10);
        glUniform1f(glGetUniformLocation(effect->programId, "maxPositiveHeight"), 30);
        glUniform1f(glGetUniformLocation(effect->programId, "baseRadius"), 100);
        check_gl_error();

        if (effect->lightMatrixId != 0xffffffff){
			glm::mat4 lm = lightMatrix * modelMatrix;
			glUniformMatrix4fv(effect->lightMatrixId, 1, GL_FALSE, &lm[0][0]);
		}

		m->bindBuffersAndDraw();

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		check_gl_error();
	}
}

// DONE create FBO for Render to texture
unsigned int createFBO(int width, int height, int nrColorBuffers, std::vector<unsigned int>& textureIds)
{
    // create Framebuffer object
	GLuint framebufferName = 0;
	glGenFramebuffers(1, &framebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
	check_gl_error();
    GLenum* drawBuffers = new GLenum[nrColorBuffers];
    
	// DONE create a texture to use as the depth buffer of the Framebuffer object
    GLuint renderedDepthTexture;
	glGenTextures(1, &renderedDepthTexture);
	glBindTexture(GL_TEXTURE_2D, renderedDepthTexture);
	// GL_DEPTH_COMPONENT is important to use the texture as depth buffer and as shadow map later
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 2, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 3, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	check_gl_error();
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// compare mode and compare func enable hardware shadow mapping. Otherwise the texture lookup would just
	// return the depth value and we would have to do the shadow comparison by ourselves
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	// configure framebuffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderedDepthTexture, 0);
	// push back the depth texture to the textureids
	// this means the depth texture is id 0!
    textureIds.push_back(renderedDepthTexture);
    
	// texture to render to - the color buffers
    for (int i = 0; i < nrColorBuffers; i++)
    {
        GLuint renderedTexture;
        glGenTextures(1, &renderedTexture);
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_HALF_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        check_gl_error();
        textureIds.push_back(renderedTexture);
		// configure the Framebuffer to use the texture as color attachment i
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, renderedTexture,0);
		// write into the drawbuffers array that the framebuffer has a color texture at attachment i
        drawBuffers[i] = GL_COLOR_ATTACHMENT0+i;
    }

	// set nrColorBuffers draw buffers for the Framebuffer object
	glDrawBuffers(nrColorBuffers, drawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("damn, end of createFBO\n");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    delete[] drawBuffers;
    return framebufferName;
}

float calculateDist(glm::vec3 center)
{
	glm::vec3 temp = center*center;
	float distance = sqrt(temp.x + temp.y + temp.z);
	return distance;
}

void initShaders(std::vector<ShaderEffect*>& shaderSets)
{
	// ########## load the shader programs ##########
    GLuint terrainGeneratorProgramId = LoadShaders("TerrainGenerator.vertexshader", "TerrainGenerator.geometryshader", "TerrainGenerator.fragmentshader", contentPath.c_str());
    SimpleShaderEffect* terrainGeneratorProgram = new SimpleShaderEffect(terrainGeneratorProgramId);
    terrainGeneratorProgram->textureSamplerId = glGetUniformLocation(terrainGeneratorProgramId, "heightSlopeBasedColorMap");
    shaderSets.push_back(terrainGeneratorProgram);
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 8);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( SCREENWIDTH, SCREENHEIGHT, "Tutorial 09 - VBO Indexing", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, SCREENWIDTH/2, SCREENHEIGHT/2);

	glm::vec3 lightPos = glm::vec3(-464,670,-570);

	// Set GLFW event callbacks. I removed glfwSetWindowSizeCallback for conciseness
	glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)TwEventMouseButtonGLFW); // - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetCursorPosCallback(window, (GLFWcursorposfun)TwEventMousePosGLFW);          // - Directly redirect GLFW mouse position events to AntTweakBar
	glfwSetScrollCallback(window, (GLFWscrollfun)TwEventMouseWheelGLFW);    // - Directly redirect GLFW mouse wheel events to AntTweakBar
	glfwSetKeyCallback(window, (GLFWkeyfun)TwEventKeyGLFW);                         // - Directly redirect GLFW key events to AntTweakBar
	glfwSetCharCallback(window, (GLFWcharfun)TwEventCharGLFW);                      // - Directly redirect GLFW char events to AntTweakBar

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SCREENWIDTH * RESOLUTION_SCALE, SCREENHEIGHT * RESOLUTION_SCALE);
	//
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	std::vector<ShaderEffect*> shaderSets;
	std::vector<RenderState*> objects;

	check_gl_error();

	initShaders(shaderSets);

	enum ShaderEffects {
		STANDARDSHADING = 0,
		TEXTURED_QUAD,
		JUST_COLOR
	};

	check_gl_error();

// ########### Load the textures ################
    GLuint heightSlopeBasedColorMap = loadSoil("beachMountain.png", contentPath.c_str());
	check_gl_error();

// ############## Load the meshes ###############
	std::vector<Mesh *> meshes;
    Mesh* sphereMesh = generateSphere(100, 7);
    sphereMesh->modelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0, 0, 0));
    meshes.push_back(sphereMesh);

    coordinateMeshIndices.clear();
	Mesh* xCoordinateMesh = generateSphere(10, 0);
	xCoordinateMesh->modelMatrix = glm::translate(glm::mat4(), glm::vec3(200, 0, 0));
    coordinateMeshIndices.push_back(meshes.size());
	meshes.push_back(xCoordinateMesh);

	Mesh* yCoordinateMesh = generateSphere(20, 0);
	yCoordinateMesh->modelMatrix = glm::translate(glm::mat4(), glm::vec3(0, 200, 0));
    coordinateMeshIndices.push_back(meshes.size());
    meshes.push_back(yCoordinateMesh);

	Mesh* zCoordinateMesh = generateSphere(30, 0);
	zCoordinateMesh->modelMatrix = glm::translate(glm::mat4(), glm::vec3(0, 0, 200));
    coordinateMeshIndices.push_back(meshes.size());
	meshes.push_back(zCoordinateMesh);
    
	for(int i = 0; i < meshes.size(); i++){
		// DONE create a SimpleRenderstate for all objects which should cast shadows
		SimpleRenderState* rtts = new SimpleRenderState();
		rtts->meshId = i;
		rtts->shaderEffectId = STANDARDSHADING; // the Render to texture shader effect
		rtts->texId = heightSlopeBasedColorMap;
		objects.push_back(rtts);
	}
	
	for (int i = 0; i < meshes.size(); i++)
	{
		meshes[i]->generateVBOs();
	}
	check_gl_error();

	// create the scenes
	enum Scenes {
		STANDARD_PASS
	};

	std::vector<Scene> scenes;
	// one scene for the standard rendering
	scenes.push_back(Scene(&objects, &meshes, &shaderSets));

	check_gl_error();


	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	do{ 
		// Apply the scene depth map to the textured quad object to debug.
		// With the gui we can change which texture we see.
		SimpleRenderState* quadObj = static_cast<SimpleRenderState*>(objects[objects.size()-1]);
		quadObj->texId = textureToShow;

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		check_gl_error();
		// Clear the screen
		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		check_gl_error();

        // compute the MVP matrix for the light
        // worldToView first
        glm::mat4 lightViewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.5, -1.0, 0.5), glm::vec3(0.0, 0.0, 1.0));
        glm::mat4 lightProjMatrix = glm::perspective(90.0f, 1.0f, 2.5f, 100.0f);
        glm::mat4 lightMVPMatrix = lightProjMatrix * lightViewMatrix;

		// set the scene constant variales ( light position)
        if(setLightToCamera)
            SimpleRenderState::lightPositionWorldSpace = getCameraPosition();
        
		// render to the screen buffer
		renderObjects(scenes[0], ViewMatrix, ProjectionMatrix, lightPos, lightMVPMatrix);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteVertexArrays(1, &VertexArrayID);


	TwTerminate();
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

