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

    float rho = 250;
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
    std::vector<unsigned int> shaderIds;
    unsigned int texId;
};

const glm::vec3 UP(0, 1, 0);
struct Camera
{
    float fieldOfView;
    float aspectRatio;
    glm::vec3 position;
    glm::vec3 targetPosition;

    glm::mat4 viewMatrix() const
    {
        return glm::lookAt(
            position,
            targetPosition,
            UP);
    }
    glm::mat4 projectionMatrix() const
    {
        return glm::perspective(fieldOfView, aspectRatio, 0.1f, 10000.0f);
    }
};

struct Scene
{
    std::vector<RenderObject> objects;
    std::vector<OpenGLMesh *> meshes;
    std::vector<ShaderEffect> shaders;
    Camera camera;

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

void update(GLFWwindow *window, Scene &scene, PlanetParameters &planetParameters, State &state)
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
        scene.lightPosition = scene.camera.position;
    }

    // clamp distance and latitude
    state.phi = glm::min(1.57f, glm::max(-1.57f, state.phi));
    state.rho = glm::min(1000.f, glm::max(10.f, state.rho));

    scene.camera.position = glm::vec3(
        state.rho * cos(state.theta) * cos(state.phi),
        state.rho * sin(state.phi),
        state.rho * sin(state.theta) * cos(state.phi));

    // Projection matrix
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    scene.camera.aspectRatio = (float)width / height;

    state.lastTime = currentTime;
}

void render(const Scene &scene, const State &state, GLuint *textures, const PlanetParameters &planetParameters)
{
    std::vector<RenderObject> objects = scene.objects;

    glPolygonMode(GL_FRONT_AND_BACK, state.wireFrameMode ? GL_LINE : GL_FILL);

    glm::vec3 lightPosition = scene.lightPosition;
    for (RenderObject rs : objects)
    {
        rs.texId = textures[state.textureIndex];
        unsigned int meshId = rs.meshId;
        OpenGLMesh *mesh = scene.meshes.at(meshId);
        glm::mat4 modelMatrix = mesh->modelMatrix;
        glm::mat4 viewMatrix = scene.camera.viewMatrix();
        glm::mat4 MVP = scene.camera.projectionMatrix() * viewMatrix * modelMatrix;

        for (int j = 0; j < rs.shaderIds.size(); j++)
        {
            unsigned int effectId = rs.shaderIds.at(j);
            ShaderEffect effect = scene.shaders.at(effectId);
            glUseProgram(effect.programId);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, rs.texId);
            glUniform1i(effect.textureSamplerId, 0);
            glUniform3f(effect.lightPositionId, lightPosition.x, lightPosition.y, lightPosition.z);

            glUniformMatrix4fv(effect.MVPId, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(effect.MId, 1, GL_FALSE, &modelMatrix[0][0]);
            glUniformMatrix4fv(effect.VId, 1, GL_FALSE, &viewMatrix[0][0]);

            glUniform1f(glGetUniformLocation(effect.programId, "maxNegativeHeight"), planetParameters.maxDepth);
            glUniform1f(glGetUniformLocation(effect.programId, "maxPositiveHeight"), planetParameters.maxHeight);
            glUniform1f(glGetUniformLocation(effect.programId, "baseRadius"), planetParameters.baseRadius);
            glUniform1f(glGetUniformLocation(effect.programId, "atmosphereRadius"), planetParameters.atmosphereRadius());
            glUniform3f(glGetUniformLocation(effect.programId, "noiseOffset"), state.noiseOffset.x, state.noiseOffset.y, state.noiseOffset.z);

            glUniform3f(glGetUniformLocation(effect.programId, "cameraPosition"), scene.camera.position.x, scene.camera.position.y, scene.camera.position.z);
            glUniform3f(glGetUniformLocation(effect.programId, "lightColor"), 1, 1, 1);

            mesh->draw();
        }
    }
    check_gl_error();
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
    Mesh atmosphereMesh = generateSphere(planetParameters.atmosphereRadius(), planetParameters.atmosphereSubdivisions);
    reverseFaces(atmosphereMesh);

    Mesh sphereMesh = generateSphere(planetParameters.baseRadius, planetParameters.planetSubdivisions);

    std::vector<OpenGLMesh *> openGlMeshes{
        new OpenGLMesh(atmosphereMesh, glm::mat4(1.0)),
        new OpenGLMesh(sphereMesh, glm::mat4(1.0))};

    std::vector<ShaderEffect> shaders{
        initializeAtmosphericScatteringShader(),
        initializeTerrainGeneratorShader(),
    };

    std::vector<RenderObject> objects{
        RenderObject{
            .meshId = 0,
            .shaderIds = std::vector<unsigned int>{0},
            .texId = textures[textureIndex],
        },
        RenderObject{
            .meshId = 1,
            .shaderIds = std::vector<unsigned int>{1},
            .texId = textures[textureIndex]}};

    Camera camera{.fieldOfView = 45.0, .targetPosition = glm::vec3(0, 0, 0)};

    return Scene{
        .meshes = openGlMeshes,
        .shaders = shaders,
        .objects = objects,
        .camera = camera};
}

class BoundVertexArrayObject
{
    GLuint vertexArrayId;

public:
    BoundVertexArrayObject()
    {
        glGenVertexArrays(1, &vertexArrayId);
        glBindVertexArray(vertexArrayId);
    }

    ~BoundVertexArrayObject()
    {
        glDeleteVertexArrays(1, &vertexArrayId);
    }
};

struct Glfw
{
    Glfw()
    {
        int initResult = glfwInit();
        if (!initResult)
        {
            throw initResult;
        }
    }

    ~Glfw()
    {
        glfwTerminate();
    }
};

struct Glew
{
    Glew()
    {
        glewExperimental = true;
        int initResult = glewInit();
        if (initResult != GLEW_OK)
        {
            throw initResult;
        }
    }
};

struct Window
{
private:
    GLFWwindow *window;

public:
    Window()
    {
        glfwWindowHint(GLFW_SAMPLES, 8);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(SCREENHEIGHT, SCREENHEIGHT, "Procedural Planets", NULL, NULL);
        if (window == NULL)
        {
            throw -1;
        }
        glfwMakeContextCurrent(window);

        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        glfwSetCursorPos(window, SCREENHEIGHT / 2, SCREENHEIGHT / 2);
    }

    ~Window()
    {
        glfwDestroyWindow(window);
    }

    GLFWwindow *glfwWindow() const
    {
        return window;
    }
};

int main(void)
{
    try
    {
        Glfw glfw;
        try
        {
            Window window;
            try
            {
                Glew glew;
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

                PlanetParameters planetParameters;
                State state;
                Scene scene = generateScene(planetParameters, textures, state.textureIndex);
                GLFWwindow *glfwWindow = window.glfwWindow();

                update(glfwWindow, scene, planetParameters, state);
                scene.lightPosition = scene.camera.position;

                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glEnable(GL_CULL_FACE);

                BoundVertexArrayObject vao;
                do
                {
                    update(glfwWindow, scene, planetParameters, state);

                    glClearColor(0.0, 0.0, 0.0, 1.0);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    render(scene, state, textures, planetParameters);

                    glfwSwapBuffers(glfwWindow);
                    glfwPollEvents();
                } while (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
                         glfwWindowShouldClose(glfwWindow) == 0);
            }
            catch (int exception)
            {
                fprintf(stderr, "Failed to initialize GLEW\n");
                return -1;
            }
        }
        catch (int exception)
        {
            fprintf(stderr, "Failed to open GLFW window.\n");
            return -1;
        }
    }
    catch (int exception)
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    return 0;
}
