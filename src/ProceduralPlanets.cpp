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
const int textureCount = 4;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "Shader.hpp"
#include "Texture.hpp"
#include "Controls.hpp"
#include "Mesh.hpp"
#include "GLError.h"
#include "RenderState.hpp"

#include "SphereGenerator.hpp"

std::string contentPath = "../shaders/";

float baseRadius = 50;
float maxDepth = 30;
float maxHeight = 40;
float seaLevelFromBaseRadius = 10;
float atmospherePlanetRatio = 0.9;
int planetMeshId;
int atmosphereMeshId;
vec3 noiseOffset = vec3(0, 0, 0);

GLuint textures[textureCount];
const char *textureNames[4] = {
    "beachMountain.png",
    "volcano.png",
    "ice.png",
    "tropic.png"};
int textureIndex = 0;

struct Scene
{
    std::vector<RenderState *> *objects;
    std::vector<Mesh *> *meshes;
    std::vector<ShaderEffect *> *effects;

    Scene(std::vector<RenderState *> *_obj,
          std::vector<Mesh *> *_meshes,
          std::vector<ShaderEffect *> *_effects) : objects(_obj),
                                                   meshes(_meshes),
                                                   effects(_effects)
    {
    }
};

void renderObjects(Scene &scene, glm::mat4x4 &viewMatrix, glm::mat4x4 &projectionMatrix, glm::vec3 &lightPos, glm::mat4 &lightMatrix)
{
    std::vector<RenderState *> *objects = scene.objects;
#ifdef MINGW_COMPILER
    glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
#else
    glm::mat4 modelMatrix = glm::mat4(1.0f);
#endif

    glPolygonMode(GL_FRONT_AND_BACK, (wireFrameMode ? GL_LINE : GL_FILL));

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glm::vec3 cameraPosition = getCameraPosition();
    check_gl_error();
    for (int i = 0; i < objects->size(); i++)
    {
        RenderState *rs = (*objects)[i];
        rs->texId = textures[textureIndex];
        unsigned int meshId = (*objects)[i]->meshId;
        Mesh *m = (*scene.meshes)[meshId];
        modelMatrix = m->modelMatrix;
        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

        for (int j = 0; j < ((*objects)[i])->shaderEffectIds.size(); j++)
        {
            // Use our shader
            unsigned int effectId = (*objects)[i]->shaderEffectIds[j];
            ShaderEffect *effect = (*scene.effects)[effectId];
            glUseProgram(effect->programId);
            check_gl_error();
            rs->setParameters(effect);
            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(effect->MVPId, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(effect->MId, 1, GL_FALSE, &modelMatrix[0][0]);
            glUniformMatrix4fv(effect->VId, 1, GL_FALSE, &viewMatrix[0][0]);
            glUniform1f(glGetUniformLocation(effect->programId, "maxNegativeHeight"), maxDepth);
            glUniform1f(glGetUniformLocation(effect->programId, "maxPositiveHeight"), maxHeight);
            glUniform1f(glGetUniformLocation(effect->programId, "baseRadius"), baseRadius);
            glUniform1f(glGetUniformLocation(effect->programId, "atmosphereRadius"), atmospherePlanetRatio * (baseRadius + maxHeight));
            glUniform3f(glGetUniformLocation(effect->programId, "cameraPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
            glUniform3f(glGetUniformLocation(effect->programId, "lightColor"), 1, 1, 1);
            glUniform3f(glGetUniformLocation(effect->programId, "noiseOffset"), noiseOffset.x, noiseOffset.y, noiseOffset.z);

            check_gl_error();

            if (effect->lightMatrixId != 0xffffffff)
            {
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
}

float calculateDist(glm::vec3 center)
{
    glm::vec3 temp = center * center;
    float distance = sqrt(temp.x + temp.y + temp.z);
    return distance;
}

void initShaders(std::vector<ShaderEffect *> &shaderSets)
{
    // ########## load the shader programs ##########
    GLuint terrainGeneratorProgramId = LoadShaders("TerrainGenerator.vertexshader", "TerrainGenerator.geometryshader", "TerrainGenerator.fragmentshader", contentPath.c_str());
    ShaderEffect *terrainGeneratorProgram = new ShaderEffect(terrainGeneratorProgramId);
    terrainGeneratorProgram->textureSamplerId = glGetUniformLocation(terrainGeneratorProgramId, "heightSlopeBasedColorMap");
    shaderSets.push_back(terrainGeneratorProgram);

    GLuint atmosphericScatteringProgramId = LoadShaders("AtmosphericScattering.vertexshader", "AtmosphericScattering.fragmentshader", contentPath.c_str());
    ShaderEffect *atmosphericScatteringProgram = new ShaderEffect(atmosphericScatteringProgramId);
    shaderSets.push_back(atmosphericScatteringProgram);
}

Scene generateScene(std::vector<Mesh *> *meshes, std::vector<RenderState *> *objects, std::vector<ShaderEffect *> *shaders)
{
    enum ShaderEffects
    {
        STANDARDSHADING = 0,
        ATMOSPHERIC_SCATTERING = 1
    };

    Mesh *atmosphereMesh = generateSphere(atmospherePlanetRatio * (baseRadius + maxHeight), 4, true);
    atmosphereMeshId = meshes->size();
    atmosphereMesh->generateVBOs();
    meshes->push_back(atmosphereMesh);
    RenderState *atmosphereRenderState = new RenderState();
    atmosphereRenderState->meshId = 0;
    atmosphereRenderState->shaderEffectIds.push_back(ATMOSPHERIC_SCATTERING);
    atmosphereRenderState->texId = textures[textureIndex];
    objects->push_back(atmosphereRenderState);

    Mesh *sphereMesh = generateSphere(baseRadius, 7, false);
    sphereMesh->generateVBOs();
    meshes->push_back(sphereMesh);
    RenderState *sphereRenderState = new RenderState();
    sphereRenderState->meshId = 1;
    sphereRenderState->shaderEffectIds.push_back(STANDARDSHADING);
    sphereRenderState->texId = textures[textureIndex];
    objects->push_back(sphereRenderState);

    return Scene(objects, meshes, shaders);
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

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetCursorPos(window, SCREENHEIGHT / 2, SCREENHEIGHT / 2);

    glm::vec3 lightPos = glm::vec3(-464, 670, -570);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    //
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);

    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    check_gl_error();
    for (int i = 0; i < textureCount; i++)
    {
        textures[i] = loadSoil(textureNames[i], "../textures/");
    }
    check_gl_error();

    std::vector<ShaderEffect *> shaders;
    initShaders(shaders);

    check_gl_error();

    std::vector<RenderState *> objects;
    std::vector<Mesh *> meshes;
    Scene scene = generateScene(&meshes, &objects, &shaders);

    check_gl_error();

    computeMatricesFromInputs();
    RenderState::lightPositionWorldSpace = getCameraPosition();

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

        glm::mat4 lightViewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.5, -1.0, 0.5), glm::vec3(0.0, 0.0, 1.0));
        glm::mat4 lightProjMatrix = glm::perspective(90.0f, 1.0f, 2.5f, 100.0f);
        glm::mat4 lightMVPMatrix = lightProjMatrix * lightViewMatrix;

        if (setLightToCamera)
        {
            RenderState::lightPositionWorldSpace = getCameraPosition();
        }

        renderObjects(scene, ViewMatrix, ProjectionMatrix, lightPos, lightMVPMatrix);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);

    glDeleteVertexArrays(1, &VertexArrayID);

    glfwTerminate();

    return 0;
}
