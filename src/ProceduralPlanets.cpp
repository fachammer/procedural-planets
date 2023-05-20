// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>

// Include GLEW
#include <GL/glew.h>

#define RESOLUTION_SCALE 1
#define SCREENWIDTH 1280
#define SCREENHEIGHT 1024

// Include GLFW
#include <glfw3.h>
GLFWwindow *window;

bool wireFrameMode = false;
bool setLightToCamera = true;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "Shader.hpp"
#include "Texture.hpp"
#include "Controls.hpp"
#include "Mesh.hpp"
#include "GLError.h"

#include "SphereGenerator.hpp"

const float baseRadius = 50;
const float maxDepth = 30;
const float maxHeight = 40;
const float seaLevelFromBaseRadius = 10;
const float atmospherePlanetRatio = 0.9;
vec3 noiseOffset = vec3(0, 0, 0);

const int textureCount = 4;
GLuint textures[textureCount];
const char *textureNames[textureCount] = {
    "beachMountain.png",
    "volcano.png",
    "ice.png",
    "tropic.png"};
int textureIndex = 0;

struct RenderObject
{
    unsigned int meshId;
    std::vector<int> shaderIds;
    unsigned int texId;
};

struct Scene
{
    std::vector<RenderObject> objects;
    std::vector<OpenGLMesh *> meshes;
    std::vector<ShaderEffect> shaders;

    glm::vec3 lightPosition;

    ~Scene()
    {
        for (OpenGLMesh *mesh : meshes)
        {
            delete mesh;
        }
    }
};

void reverseFaces(Mesh &mesh)
{
    std::vector<unsigned int> reversedIndices;

    for (int i = 0; i < mesh.indices.size(); i += 3)
    {
        for (int j = 2; j >= 0; j--)
        {
            reversedIndices.push_back(mesh.indices[i + j]);
        }
    }

    mesh.indices = reversedIndices;
}

void renderScene(const Scene &scene, glm::mat4x4 &viewMatrix, const glm::mat4x4 &projectionMatrix)
{
    std::vector<RenderObject> objects = scene.objects;

    glPolygonMode(GL_FRONT_AND_BACK, (wireFrameMode ? GL_LINE : GL_FILL));

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glm::vec3 cameraPosition = getCameraPosition();
    glm::vec3 lightPosition = scene.lightPosition;
    check_gl_error();
    for (RenderObject rs : objects)
    {
        rs.texId = textures[textureIndex];
        unsigned int meshId = rs.meshId;
        OpenGLMesh *mesh = scene.meshes.at(meshId);
        glm::mat4 modelMatrix = mesh->modelMatrix;
        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

        for (int j = 0; j < rs.shaderIds.size(); j++)
        {
            // Use our shader
            unsigned int effectId = rs.shaderIds.at(j);
            ShaderEffect effect = scene.shaders.at(effectId);
            glUseProgram(effect.programId);
            check_gl_error();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, rs.texId);
            glUniform1i(effect.textureSamplerId, 0);
            check_gl_error();
            glUniform3f(effect.lightPositionId, lightPosition.x, lightPosition.y, lightPosition.z);

            glUniformMatrix4fv(effect.MVPId, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(effect.MId, 1, GL_FALSE, &modelMatrix[0][0]);
            glUniformMatrix4fv(effect.VId, 1, GL_FALSE, &viewMatrix[0][0]);

            glUniform1f(glGetUniformLocation(effect.programId, "maxNegativeHeight"), maxDepth);
            glUniform1f(glGetUniformLocation(effect.programId, "maxPositiveHeight"), maxHeight);
            glUniform1f(glGetUniformLocation(effect.programId, "baseRadius"), baseRadius);
            glUniform1f(glGetUniformLocation(effect.programId, "atmosphereRadius"), atmospherePlanetRatio * (baseRadius + maxHeight));
            glUniform3f(glGetUniformLocation(effect.programId, "cameraPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
            glUniform3f(glGetUniformLocation(effect.programId, "lightColor"), 1, 1, 1);
            glUniform3f(glGetUniformLocation(effect.programId, "noiseOffset"), noiseOffset.x, noiseOffset.y, noiseOffset.z);

            check_gl_error();

            mesh->draw();

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);
            glDisableVertexAttribArray(3);
            glDisableVertexAttribArray(4);
            check_gl_error();
        }
    }
}

ShaderEffect initializeTerrainGeneratorShader()
{
    GLuint terrainGeneratorProgramId = LoadShaders("../shaders/TerrainGenerator.vertexshader", "../shaders/TerrainGenerator.geometryshader", "../shaders/TerrainGenerator.fragmentshader");
    ShaderEffect terrainGeneratorProgram = ShaderEffect(terrainGeneratorProgramId);
    terrainGeneratorProgram.textureSamplerId = glGetUniformLocation(terrainGeneratorProgramId, "heightSlopeBasedColorMap");
    return terrainGeneratorProgram;
}

ShaderEffect initializeAtmosphericScatteringShader()
{
    GLuint atmosphericScatteringProgramId = LoadShaders("../shaders/AtmosphericScattering.vertexshader", "../shaders/AtmosphericScattering.fragmentshader");
    ShaderEffect atmosphericScatteringProgram = ShaderEffect(atmosphericScatteringProgramId);
    return atmosphericScatteringProgram;
}

Scene generateScene()
{
    Scene scene;

    ShaderEffect atmosphereShader = initializeAtmosphericScatteringShader();
    scene.shaders.push_back(atmosphereShader);
    Mesh atmosphereMesh = generateSphere(atmospherePlanetRatio * (baseRadius + maxHeight), 4);
    reverseFaces(atmosphereMesh);
    scene.meshes.push_back(new OpenGLMesh(atmosphereMesh, glm::mat4(1.0)));
    RenderObject atmosphereRenderState = RenderObject();
    atmosphereRenderState.meshId = scene.meshes.size() - 1;
    atmosphereRenderState.shaderIds.push_back(scene.shaders.size() - 1);
    atmosphereRenderState.texId = textures[textureIndex];
    scene.objects.push_back(atmosphereRenderState);

    ShaderEffect terrainGeneratorShader = initializeTerrainGeneratorShader();
    scene.shaders.push_back(terrainGeneratorShader);
    Mesh sphereMesh = generateSphere(baseRadius, 7);
    scene.meshes.push_back(new OpenGLMesh(sphereMesh, glm::mat4(1.0)));
    RenderObject sphereRenderState = RenderObject();
    sphereRenderState.meshId = scene.meshes.size() - 1;
    sphereRenderState.shaderIds.push_back(scene.shaders.size() - 1);
    sphereRenderState.texId = textures[textureIndex];
    scene.objects.push_back(sphereRenderState);

    return scene;
}

int main(void)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(SCREENHEIGHT, SCREENHEIGHT, "Procedural Planets", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    check_gl_error();

    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    check_gl_error();

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetCursorPos(window, SCREENHEIGHT / 2, SCREENHEIGHT / 2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    check_gl_error();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    check_gl_error();
    for (int i = 0; i < textureCount; i++)
    {
        textures[i] = loadSoil(textureNames[i], "../textures/");
    }
    check_gl_error();

    Scene scene = generateScene();

    check_gl_error();

    computeMatricesFromInputs();
    scene.lightPosition = getCameraPosition();

    srand(time(NULL));

    do
    {
        check_gl_error();
        // Clear the screen
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        check_gl_error();

        if (setLightToCamera)
        {
            scene.lightPosition = getCameraPosition();
        }

        renderScene(scene, ViewMatrix, ProjectionMatrix);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);

    glDeleteVertexArrays(1, &VertexArrayID);

    glfwTerminate();

    return 0;
}
