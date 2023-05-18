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

#include "shader.hpp"
#include "texture.hpp"
#include "controls.hpp"
#include "Mesh.hpp"
#include "GLError.h"
#include "RenderState.hpp"

#include "SphereGenerator.hpp"

#define PATHTOCONTENT "../src/"

// Create and compile our GLSL program from the shaders
std::string contentPath = PATHTOCONTENT;

float shadowMagicNumber = 0.003;
unsigned char textureToShow = 0;
unsigned char layerToShow = 0;
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

std::vector<unsigned int> coordinateMeshIndices;
bool drawCoordinateMeshes = false;
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
        if (!drawCoordinateMeshes && std::find(coordinateMeshIndices.begin(), coordinateMeshIndices.end(), i) != coordinateMeshIndices.end())
            continue;

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
    SimpleShaderEffect *terrainGeneratorProgram = new SimpleShaderEffect(terrainGeneratorProgramId);
    terrainGeneratorProgram->textureSamplerId = glGetUniformLocation(terrainGeneratorProgramId, "heightSlopeBasedColorMap");
    shaderSets.push_back(terrainGeneratorProgram);

    GLuint atmosphericScatteringProgramId = LoadShaders("AtmosphericScattering.vertexshader", "AtmosphericScattering.fragmentshader", contentPath.c_str());
    SimpleShaderEffect *atmosphericScatteringProgram = new SimpleShaderEffect(atmosphericScatteringProgramId);
    shaderSets.push_back(atmosphericScatteringProgram);
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
    window = glfwCreateWindow(SCREENHEIGHT, SCREENHEIGHT, "Tutorial 09 - VBO Indexing", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

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

    std::vector<ShaderEffect *> shaderSets;
    std::vector<RenderState *> objects;

    check_gl_error();

    initShaders(shaderSets);

    enum ShaderEffects
    {
        STANDARDSHADING = 0,
        ATMOSPHERIC_SCATTERING = 1
    };

    check_gl_error();

    // ########### Load the textures ################
    for (int i = 0; i < textureCount; i++)
    {
        textures[i] = loadSoil(textureNames[i], "../textures/");
    }
    check_gl_error();

    // ############## Load the meshes ###############
    std::vector<Mesh *> meshes;

    Mesh *atmosphereMesh = generateSphere(atmospherePlanetRatio * (baseRadius + maxHeight), 7, true);
    atmosphereMeshId = meshes.size();
    meshes.push_back(atmosphereMesh);

    Mesh *sphereMesh = generateSphere(baseRadius, 7, false);
    planetMeshId = meshes.size();
    meshes.push_back(sphereMesh);

    for (int i = 0; i < meshes.size(); i++)
    {
        // DONE create a Renderstate for all objects which should cast shadows
        RenderState *rtts = new RenderState();
        rtts->meshId = i;
        if (i == atmosphereMeshId)
            rtts->shaderEffectIds.push_back(ATMOSPHERIC_SCATTERING);
        else
            rtts->shaderEffectIds.push_back(STANDARDSHADING);
        rtts->texId = textures[textureIndex];
        objects.push_back(rtts);
    }

    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i]->generateVBOs();
    }
    check_gl_error();

    // create the scenes
    std::vector<Scene> scenes;
    // one scene for the standard rendering
    scenes.push_back(Scene(&objects, &meshes, &shaderSets));

    check_gl_error();

    computeMatricesFromInputs();
    RenderState::lightPositionWorldSpace = getCameraPosition();
    RenderState::lightPositionWorldSpace2 = glm::vec3(0, 0, 0) - (3.0f * getCameraPosition());

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    srand(time(NULL));

    do
    {
        // Apply the scene depth map to the textured quad object to debug.
        // With the gui we can change which texture we see.
        RenderState *quadObj = static_cast<RenderState *>(objects[objects.size() - 1]);
        quadObj->texId = textureToShow;

        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0)
        { // If last prinf() was more than 1sec ago
            // printf and reset
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        check_gl_error();
        // Clear the screen
        glClearColor(0.0, 0.0, 0.0, 1.0);
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
        if (setLightToCamera)
        {
            RenderState::lightPositionWorldSpace = getCameraPosition();
            RenderState::lightPositionWorldSpace2 = glm::vec3(0, 0, 0) - (3.0f * getCameraPosition());
        }

        // render to the screen buffer
        renderObjects(scenes[0], ViewMatrix, ProjectionMatrix, lightPos, lightMVPMatrix);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    // Cleanup VBO and shader
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
