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

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include "GLError.h"

#include "SphereGenerator.hpp"

struct PlanetParameters
{
    const float baseRadius = 50;
    const float maxDepth = 30;
    const float maxHeight = 40;
    const float seaLevelFromBaseRadius = 10;
    const float atmospherePlanetRatio = 0.9;
    const unsigned int atmosphereSubdivisions = 4;
    const unsigned int planetSubdivisions = 7;

    float atmosphereRadius() const
    {
        return atmospherePlanetRatio * (baseRadius + maxHeight);
    }
};

struct State
{
    const float defaultSpeed = 0.75f;
    float speed = 300.f;
    const float defaultRotateSpeed = 0.003f;
    float rotateSpeed = 1.5f;

    float rho = 500;
    float theta = 0;
    float phi = 0;

    glm::vec3 noiseOffset = glm::vec3(0, 0, 0);
    bool canChangeWireframeMode = true;
    bool canChangeDrawCoordinateMeshes = true;
    bool canGenerateNewNoise = true;

    bool wireFrameMode = false;

    int textureIndex = 0;

    float lastTime = 0;
};

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

struct Camera
{
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 position;

    float fieldOfView = 45.0f;

    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 targetPos = glm::vec3(0, 0, 0);
};

void update(GLFWwindow *window, Scene &scene, Camera &camera, PlanetParameters &planetParameters, State &state)
{
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - state.lastTime);

    // update move speed based on distance
    state.rotateSpeed = state.defaultRotateSpeed * state.rho;
    state.speed = state.defaultSpeed * state.rho;

    // change latitude
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        state.phi += deltaTime * state.rotateSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        state.phi -= deltaTime * state.rotateSpeed;
    }

    // change longitude
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        state.theta -= deltaTime * state.rotateSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        state.theta += deltaTime * state.rotateSpeed;
    }

    // Move towards and away from planet
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        state.rho -= state.speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        state.rho += state.speed * deltaTime;
    }

    // toggle wireframe mode
    int changeMode = glfwGetKey(window, GLFW_KEY_F);
    if (changeMode == GLFW_PRESS && state.canChangeWireframeMode)
    {
        state.wireFrameMode = !state.wireFrameMode;
        state.canChangeWireframeMode = false;
    }
    else if (changeMode == GLFW_RELEASE)
    {
        state.canChangeWireframeMode = true;
    }

    int newNoiseOffset = glfwGetKey(window, GLFW_KEY_R);
    if (newNoiseOffset == GLFW_PRESS && state.canGenerateNewNoise)
    {
        state.noiseOffset = glm::vec3(rand() % 99, rand() % 99, rand() % 99);
        state.canGenerateNewNoise = false;
    }
    else if (newNoiseOffset == GLFW_RELEASE)
    {
        state.canGenerateNewNoise = true;
    }

    for (int i = 0; i < 4; i++)
    {
        if (glfwGetKey(window, GLFW_KEY_1 + i) == GLFW_PRESS)
        {
            state.textureIndex = i;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        scene.lightPosition = camera.position;
    }

    // clamp distance and latitude
    state.phi = glm::min(1.57f, glm::max(-1.57f, state.phi));
    state.rho = glm::min(1000.f, glm::max(10.f, state.rho));

    camera.position = glm::vec3(
        state.rho * cos(state.theta) * cos(state.phi),
        state.rho * sin(state.phi),
        state.rho * sin(state.theta) * cos(state.phi));

    // Projection matrix
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    camera.projectionMatrix = glm::perspective(camera.fieldOfView, (float)width / height, 0.1f, 10000.0f);
    // Camera matrix
    camera.viewMatrix = glm::lookAt(
        camera.position,
        camera.targetPos,
        camera.up);

    state.lastTime = currentTime;
}

void render(GLFWwindow *window, const Scene &scene, const State &state, const Camera &camera, GLuint *textures, const PlanetParameters &planetParameters)
{
    std::vector<RenderObject> objects = scene.objects;

    glPolygonMode(GL_FRONT_AND_BACK, state.wireFrameMode ? GL_LINE : GL_FILL);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glm::vec3 lightPosition = scene.lightPosition;
    check_gl_error();
    for (RenderObject rs : objects)
    {
        rs.texId = textures[state.textureIndex];
        unsigned int meshId = rs.meshId;
        OpenGLMesh *mesh = scene.meshes.at(meshId);
        glm::mat4 modelMatrix = mesh->modelMatrix;
        glm::mat4 MVP = camera.projectionMatrix * camera.viewMatrix * modelMatrix;

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
            glUniformMatrix4fv(effect.VId, 1, GL_FALSE, &camera.viewMatrix[0][0]);

            glUniform1f(glGetUniformLocation(effect.programId, "maxNegativeHeight"), planetParameters.maxDepth);
            glUniform1f(glGetUniformLocation(effect.programId, "maxPositiveHeight"), planetParameters.maxHeight);
            glUniform1f(glGetUniformLocation(effect.programId, "baseRadius"), planetParameters.baseRadius);
            glUniform1f(glGetUniformLocation(effect.programId, "atmosphereRadius"), planetParameters.atmosphereRadius());
            glUniform3f(glGetUniformLocation(effect.programId, "noiseOffset"), state.noiseOffset.x, state.noiseOffset.y, state.noiseOffset.z);

            glUniform3f(glGetUniformLocation(effect.programId, "cameraPosition"), camera.position.x, camera.position.y, camera.position.z);
            glUniform3f(glGetUniformLocation(effect.programId, "lightColor"), 1, 1, 1);

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

Scene generateScene(const PlanetParameters &planetParameters, GLuint *textures, unsigned int textureIndex)
{
    Scene scene;

    ShaderEffect atmosphereShader = initializeAtmosphericScatteringShader();
    scene.shaders.push_back(atmosphereShader);
    Mesh atmosphereMesh = generateSphere(planetParameters.atmosphereRadius(), planetParameters.atmosphereSubdivisions);
    reverseFaces(atmosphereMesh);
    scene.meshes.push_back(new OpenGLMesh(atmosphereMesh, glm::mat4(1.0)));
    RenderObject atmosphereRenderState = RenderObject();
    atmosphereRenderState.meshId = scene.meshes.size() - 1;
    atmosphereRenderState.shaderIds.push_back(scene.shaders.size() - 1);
    atmosphereRenderState.texId = textures[textureIndex];
    scene.objects.push_back(atmosphereRenderState);

    ShaderEffect terrainGeneratorShader = initializeTerrainGeneratorShader();
    scene.shaders.push_back(terrainGeneratorShader);
    Mesh sphereMesh = generateSphere(planetParameters.baseRadius, planetParameters.planetSubdivisions);
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

    GLFWwindow *window = glfwCreateWindow(SCREENHEIGHT, SCREENHEIGHT, "Procedural Planets", NULL, NULL);
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
    const int textureCount = 4;
    GLuint textures[textureCount];
    const char *textureNames[textureCount] = {
        "beachMountain.png",
        "volcano.png",
        "ice.png",
        "tropic.png"};

    for (int i = 0; i < textureCount; i++)
    {
        textures[i] = loadSoil(textureNames[i], "../textures/");
    }

    check_gl_error();

    PlanetParameters planetParameters;
    State state;
    Scene scene = generateScene(planetParameters, textures, state.textureIndex);
    Camera camera;
    update(window, scene, camera, planetParameters, state);
    scene.lightPosition = camera.position;

    srand(time(NULL));

    do
    {
        update(window, scene, camera, planetParameters, state);

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render(window, scene, state, camera, textures, planetParameters);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);

    glDeleteVertexArrays(1, &VertexArrayID);

    glfwTerminate();

    return 0;
}
