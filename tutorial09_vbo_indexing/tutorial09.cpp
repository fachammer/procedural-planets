// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>



// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

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

#define PATHTOCONTENT "../tutorial09_vbo_indexing/"

struct RenderState
{
	Mesh* mesh;
	ShaderProgram* shaderProgram;
	unsigned int textureId;
	unsigned int texture2Id;
};

void fins(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,

	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
){
	for( int i = 0; i < in_vertices.size(); i+=3)
	{
		glm::vec3 one = in_vertices[i];			//glm::vec2 oneUv = in_uvs[i];
		glm::vec3 two = in_vertices[i+1];		//glm::vec2 twoUv = in_uvs[i];
		glm::vec3 three = in_vertices[i+2];		//glm::vec2 threeUv = in_uvs[i];

		glm::vec3 small = glm::vec3(0.1, 0.1, 0.1);
		glm::vec3 none = glm::normalize(in_normals[i])* small;
		glm::vec3 ntwo = glm::normalize(in_normals[i+1])* small;
		glm::vec3 nthree = glm::normalize(in_normals[i+2])* small;

		glm::vec3 oneh = one + none;
		glm::vec3 twoh = two + ntwo;
		glm::vec3 threeh = three + nthree;

		out_vertices.push_back(one); out_uvs.push_back(glm::vec2(0.0f, 0.0f));	out_normals.push_back(in_normals[i]);
		out_vertices.push_back(two); out_uvs.push_back(glm::vec2(1.0f, 0.0f));	out_normals.push_back(in_normals[i+1]);
		out_vertices.push_back(oneh); out_uvs.push_back(glm::vec2(0.0f, 1.0f)); out_normals.push_back(in_normals[i]);
		out_vertices.push_back(oneh); out_uvs.push_back(glm::vec2(0.0f, 1.0f));	out_normals.push_back(in_normals[i]);
		out_vertices.push_back(two); out_uvs.push_back(glm::vec2(1.0f, 0.0f));	out_normals.push_back(in_normals[i+1]);
		out_vertices.push_back(twoh); out_uvs.push_back(glm::vec2(1.0f, 1.0f));	out_normals.push_back(in_normals[i+1]);

		out_vertices.push_back(two); out_uvs.push_back(glm::vec2(0.0f, 0.0f));	out_normals.push_back(in_normals[i+1]);
		out_vertices.push_back(three); out_uvs.push_back(glm::vec2(1.0f, 0.0f));	out_normals.push_back(in_normals[i+2]);
		out_vertices.push_back(twoh); out_uvs.push_back(glm::vec2(0.0f, 1.0f)); out_normals.push_back(in_normals[i+1]);
		out_vertices.push_back(twoh); out_uvs.push_back(glm::vec2(0.0f, 1.0f));	out_normals.push_back(in_normals[i+1]);
		out_vertices.push_back(three); out_uvs.push_back(glm::vec2(1.0f, 0.0f));	out_normals.push_back(in_normals[i+2]);
		out_vertices.push_back(threeh); out_uvs.push_back(glm::vec2(1.0f, 1.0f));	out_normals.push_back(in_normals[i+2]);

		out_vertices.push_back(three); out_uvs.push_back(glm::vec2(0.0f, 0.0f));	out_normals.push_back(in_normals[i+2]);
		out_vertices.push_back(one); out_uvs.push_back(glm::vec2(1.0f, 0.0f));	out_normals.push_back(in_normals[i]);
		out_vertices.push_back(threeh); out_uvs.push_back(glm::vec2(0.0f, 1.0f)); out_normals.push_back(in_normals[i+2]);
		out_vertices.push_back(threeh); out_uvs.push_back(glm::vec2(0.0f, 1.0f));	out_normals.push_back(in_normals[i+2]);
		out_vertices.push_back(one); out_uvs.push_back(glm::vec2(1.0f, 0.0f));	out_normals.push_back(in_normals[i]);
		out_vertices.push_back(oneh); out_uvs.push_back(glm::vec2(1.0f, 1.0f));	out_normals.push_back(in_normals[i]);
	}
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 09 - VBO Indexing", NULL, NULL);
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
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	//
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
	//

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	//

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	std::vector<Mesh> meshes;
	std::vector<ShaderProgram> shaderSets;
	std::vector<RenderState> objects;

	////

	// Create and compile our GLSL program from the shaders
	std::string contentPath = PATHTOCONTENT;

	std::string vertexShaderPath = contentPath;
	vertexShaderPath += std::string("StandardShading2.vertexshader");//contentPath.append("StandardShading.vertexshader");
	std::string fragmentShaderPath = contentPath;
	fragmentShaderPath += std::string("StandardShading.fragmentshader");
	GLuint renderingProgramID = LoadShaders(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
	ShaderProgram renderingProgram = ShaderProgram(renderingProgramID);
	shaderSets.push_back(renderingProgram);

	//

	// Load the texture
	std::string texturePath = contentPath;
	texturePath += std::string("spongebob.DDS");
	GLuint Texture1 = loadDDS(texturePath.c_str());

	texturePath = contentPath;
	texturePath += std::string("lichenStone.dds");
	GLuint Texture2 = loadDDS(texturePath.c_str());
	//GLuint Texture2 = loadBMP_custom(texturePath.c_str());

	texturePath = contentPath;
	texturePath += std::string("testLighting.bmp");
	//GLuint Texture3 = loadDDS(texturePath.c_str());
	GLuint Texture3 = loadBMP_custom(texturePath.c_str());

	// Read our .obj file
	std::string modelPath = contentPath;
	modelPath += std::string("Landscape.obj");
	Mesh m;
	bool res = m.loadFromOBJ(modelPath.c_str());//loadOBJ(modelPath.c_str(), vertices, uvs, normals);
	m.calculateTangents();
	m.indexMesh();
	m.generateVBOs();
	m.shaderProgram = &renderingProgram;

	modelPath = contentPath;
	modelPath += std::string("Stone1.obj");
	Mesh landMesh;
	res = landMesh.loadFromOBJ(modelPath.c_str());//loadOBJ(modelPath.c_str(), vertices, uvs, normals);
	landMesh.calculateTangents();
	landMesh.indexMesh();
	landMesh.generateVBOs();
	landMesh.shaderProgram = &renderingProgram;

	modelPath = contentPath;
	modelPath += std::string("MiniChassis.obj");
	Mesh miniMesh;
	res = miniMesh.loadFromOBJ(modelPath.c_str());//loadOBJ(modelPath.c_str(), vertices, uvs, normals);
	miniMesh.calculateTangents();
	miniMesh.indexMesh();
	miniMesh.generateVBOs();
	miniMesh.shaderProgram = &renderingProgram;

	meshes.push_back(landMesh);

	RenderState solidObject = {&m, &shaderSets[0], Texture2, Texture3};
	RenderState landscapeObject = {&landMesh, &shaderSets[0], Texture2, Texture3};
	RenderState miniObject = {&miniMesh, &shaderSets[0], Texture3, Texture3};
	objects.push_back(solidObject);
	objects.push_back(miniObject);
	objects.push_back(landscapeObject);

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	do{

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		
		for(int i = 0; i < objects.size(); i++)
		{
			Mesh* m = (objects[i].mesh);
			// Use our shader
			glUseProgram(objects[i].shaderProgram->programId);

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(objects[i].shaderProgram->MVPId, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(objects[i].shaderProgram->MId, 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(objects[i].shaderProgram->VId, 1, GL_FALSE, &ViewMatrix[0][0]);

			glm::vec3 lightPos = glm::vec3(4,4,4);
			glUniform3f(objects[i].shaderProgram->lightPositionId, lightPos.x, lightPos.y, lightPos.z);

			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, objects[i].textureId);
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(m->shaderProgram->textureSamplerId, 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, objects[i].texture2Id);
			glUniform1i(m->shaderProgram->textureSampler2Id, 1);

			m->bindBuffersAndDraw();
			
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(3);
			glDisableVertexAttribArray(4);

		}

		

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	//glDeleteProgram(programID);
	glDeleteTextures(1, &Texture1);
	glDeleteTextures(1, &Texture2);
	glDeleteTextures(1, &Texture3);
	glDeleteVertexArrays(1, &VertexArrayID);

	
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

