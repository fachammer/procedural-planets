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
#include "Mesh.hpp"
#include "GLError.h"

#include "SphereGenerator.hpp"
#include <SOIL.h>

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
    float fieldOfView = 45.0;
    float distanceFromOrigin = 250;
    float azimuthalAngle = 0;
    float polarAngle = 0;

    glm::vec3 position() const
    {
        return glm::vec3(
            distanceFromOrigin * cos(azimuthalAngle) * cos(polarAngle),
            distanceFromOrigin * sin(polarAngle),
            distanceFromOrigin * sin(azimuthalAngle) * cos(polarAngle));
    }

    glm::mat4 viewMatrix() const
    {
        return glm::lookAt(
            position(),
            glm::vec3(0, 0, 0),
            UP);
    }
};

class Texture
{
private:
    GLuint textureId = 0;

    void dispose()
    {
        glDeleteTextures(1, &textureId);
        invalidate();
    }

    void invalidate()
    {
        textureId = 0;
    }

public:
    Texture();
    Texture(std::string path)
    {
        textureId = SOIL_load_OGL_texture(
            path.c_str(),
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    Texture(Texture &&other) : textureId(other.textureId)
    {
        other.invalidate();
    }

    Texture &operator=(Texture &&other)
    {
        if (this != &other)
        {
            dispose();
            std::swap(textureId, other.textureId);
        }
        return *this;
    }

    ~Texture()
    {
        dispose();
    }

    GLuint id() const
    {
        return textureId;
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
ShaderEffect initializeTerrainGeneratorShader()
{
    GLuint terrainGeneratorProgramId = LoadShaders("../shaders/TerrainGenerator.vertex.glsl", "../shaders/TerrainGenerator.fragment.glsl");
    ShaderEffect terrainGeneratorProgram = ShaderEffect(terrainGeneratorProgramId);
    terrainGeneratorProgram.textureSamplerId = glGetUniformLocation(terrainGeneratorProgramId, "heightSlopeBasedColorMap");
    return terrainGeneratorProgram;
}

ShaderEffect initializeAtmosphericScatteringShader()
{
    GLuint atmosphericScatteringProgramId = LoadShaders("../shaders/AtmosphericScattering.vertex.glsl", "../shaders/AtmosphericScattering.fragment.glsl");
    ShaderEffect atmosphericScatteringProgram = ShaderEffect(atmosphericScatteringProgramId);
    return atmosphericScatteringProgram;
}

struct Scene
{
    std::vector<RenderObject> objects;
    std::vector<OpenGLMesh *> meshes;
    std::vector<ShaderEffect> shaders;
    std::vector<Texture *> textures;

    Camera camera;
    glm::vec3 lightPosition;

    State state;
    PlanetParameters planetParameters;

    Scene()
    {
        Mesh atmosphereMesh = generateSphere(planetParameters.atmosphereRadius(), planetParameters.atmosphereSubdivisions);
        reverseFaces(atmosphereMesh);

        Mesh sphereMesh = generateSphere(planetParameters.baseRadius, planetParameters.planetSubdivisions);

        meshes = {
            new OpenGLMesh(atmosphereMesh, glm::mat4(1.0)),
            new OpenGLMesh(sphereMesh, glm::mat4(1.0))};

        shaders = {
            initializeAtmosphericScatteringShader(),
            initializeTerrainGeneratorShader(),
        };

        textures = {
            new Texture("../textures/beachMountain.png"),
            new Texture("../textures/ice.png"),
            new Texture("../textures/tropic.png"),
            new Texture("../textures/volcano.png")};

        objects = {
            RenderObject{
                .meshId = 0,
                .shaderIds = std::vector<unsigned int>{0},
                .texId = textures[state.textureIndex]->id(),
            },
            RenderObject{
                .meshId = 1,
                .shaderIds = std::vector<unsigned int>{1},
                .texId = textures[state.textureIndex]->id()}};

        lightPosition = camera.position();
    }

    ~Scene()
    {
        for (OpenGLMesh *mesh : meshes)
        {
            delete mesh;
        }

        for (Texture *texture : textures)
        {
            delete texture;
        }
    }
};

void update(GLFWwindow *window, Scene &scene)
{
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - scene.state.lastTime);

    scene.state.rotateSpeed = scene.state.defaultRotateSpeed * scene.camera.distanceFromOrigin;
    scene.state.speed = scene.state.defaultSpeed * scene.camera.distanceFromOrigin;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        scene.camera.polarAngle += deltaTime * scene.state.rotateSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        scene.camera.polarAngle -= deltaTime * scene.state.rotateSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        scene.camera.azimuthalAngle -= deltaTime * scene.state.rotateSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        scene.camera.azimuthalAngle += deltaTime * scene.state.rotateSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        scene.camera.distanceFromOrigin -= scene.state.speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        scene.camera.distanceFromOrigin += scene.state.speed * deltaTime;
    }

    scene.camera.polarAngle = glm::min(1.57f, glm::max(-1.57f, scene.camera.polarAngle));
    scene.camera.distanceFromOrigin = glm::min(1000.f, glm::max(10.f, scene.camera.distanceFromOrigin));

    int changeMode = glfwGetKey(window, GLFW_KEY_F);
    if (changeMode == GLFW_PRESS && scene.state.canChangeWireframeMode)
    {
        scene.state.wireFrameMode = !scene.state.wireFrameMode;
        scene.state.canChangeWireframeMode = false;
    }
    else if (changeMode == GLFW_RELEASE)
    {
        scene.state.canChangeWireframeMode = true;
    }

    int newNoiseOffset = glfwGetKey(window, GLFW_KEY_R);
    if (newNoiseOffset == GLFW_PRESS && scene.state.canGenerateNewNoise)
    {
        scene.state.noiseOffset = glm::vec3(rand() % 99, rand() % 99, rand() % 99);
        scene.state.canGenerateNewNoise = false;
    }
    else if (newNoiseOffset == GLFW_RELEASE)
    {
        scene.state.canGenerateNewNoise = true;
    }

    for (int i = 0; i < 4; i++)
    {
        if (glfwGetKey(window, GLFW_KEY_1 + i) == GLFW_PRESS)
        {
            scene.state.textureIndex = i;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        scene.lightPosition = scene.camera.position();
    }

    scene.state.lastTime = currentTime;
}

void render(GLFWwindow *glfwWindow, const Scene &scene)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, scene.state.wireFrameMode ? GL_LINE : GL_FILL);

    int width, height;
    glfwGetWindowSize(glfwWindow, &width, &height);
    float aspectRatio = (float)width / height;
    glm::vec3 cameraPosition = scene.camera.position();
    for (RenderObject renderObject : scene.objects)
    {
        renderObject.texId = scene.textures[scene.state.textureIndex]->id();
        unsigned int meshId = renderObject.meshId;
        OpenGLMesh *mesh = scene.meshes.at(meshId);
        glm::mat4 modelMatrix = mesh->modelMatrix;
        glm::mat4 viewMatrix = scene.camera.viewMatrix();
        glm::mat4 projectionMatrix = glm::perspective(scene.camera.fieldOfView, aspectRatio, 0.1f, 10000.0f);
        glm::mat4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;

        for (int j = 0; j < renderObject.shaderIds.size(); j++)
        {
            unsigned int effectId = renderObject.shaderIds.at(j);
            ShaderEffect effect = scene.shaders.at(effectId);
            glUseProgram(effect.programId);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, renderObject.texId);
            glUniform1i(effect.textureSamplerId, 0);
            glUniform3f(effect.lightPositionId, scene.lightPosition.x, scene.lightPosition.y, scene.lightPosition.z);

            glUniformMatrix4fv(effect.MVPId, 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);
            glUniformMatrix4fv(effect.MId, 1, GL_FALSE, &modelMatrix[0][0]);
            glUniformMatrix4fv(effect.VId, 1, GL_FALSE, &viewMatrix[0][0]);

            glUniform1f(glGetUniformLocation(effect.programId, "maxNegativeHeight"), scene.planetParameters.maxDepth);
            glUniform1f(glGetUniformLocation(effect.programId, "maxPositiveHeight"), scene.planetParameters.maxHeight);
            glUniform1f(glGetUniformLocation(effect.programId, "baseRadius"), scene.planetParameters.baseRadius);
            glUniform1f(glGetUniformLocation(effect.programId, "atmosphereRadius"), scene.planetParameters.atmosphereRadius());
            glUniform3f(glGetUniformLocation(effect.programId, "noiseOffset"), scene.state.noiseOffset.x, scene.state.noiseOffset.y, scene.state.noiseOffset.z);

            glUniform3f(glGetUniformLocation(effect.programId, "cameraPositionInWorldSpace"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
            glUniform3f(glGetUniformLocation(effect.programId, "lightColor"), 1, 1, 1);

            mesh->draw();
        }
    }

    glfwSwapBuffers(glfwWindow);
    glfwPollEvents();
    check_gl_error();
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

    Glfw(const Glfw &) = delete;
    Glfw operator=(const Glfw &) = delete;

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

                Scene scene;
                GLFWwindow *glfwWindow = window.glfwWindow();

                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glEnable(GL_CULL_FACE);

                BoundVertexArrayObject vao;
                do
                {
                    update(glfwWindow, scene);
                    render(glfwWindow, scene);

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
