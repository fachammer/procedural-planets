// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>
#include <memory>

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
    float baseRadius = 50;
    float maxDepth = 30;
    float maxHeight = 40;
    float seaLevelFromBaseRadius = 10;
    float atmospherePlanetRatio = 0.9;
    unsigned int atmosphereSubdivisions = 4;
    unsigned int planetSubdivisions = 7;

    float atmosphereRadius() const
    {
        return atmospherePlanetRatio * (baseRadius + maxHeight);
    }
};

struct State
{
    float defaultSpeed = 0.75f;
    float speed = 300.f;
    float rotateSpeed = 1.5f;
    float defaultRotateSpeed = 0.003f;

    glm::vec3 noiseOffset = glm::vec3(0, 0, 0);
    bool canChangeWireframeMode = true;
    bool canChangeDrawCoordinateMeshes = true;
    bool canGenerateNewNoise = true;

    bool wireFrameMode = false;

    int textureIndex = 0;

    float lastTime = 0;
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
    GLuint textureId;

public:
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
        other.textureId = 0;
    }

    Texture &operator=(Texture &&other)
    {
        if (this != &other)
        {
            textureId = other.textureId;
            other.textureId = 0;
        }
        return *this;
    }

    ~Texture()
    {
        glDeleteTextures(1, &textureId);
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

struct Scene
{
    struct RenderObject
    {
        unsigned int meshIndex;
        std::vector<unsigned int> shaderIndices;
        unsigned int textureIndex;
    };
    std::vector<RenderObject> objects;
    std::vector<OpenGLMesh> meshes;
    std::vector<ShaderProgram> shaderPrograms;
    std::vector<Texture> textures;

    Camera camera;
    glm::vec3 lightDirection;

    State state;
    PlanetParameters planetParameters;

    Scene()
    {
        Mesh atmosphereMesh = generateSphere(planetParameters.atmosphereRadius(), planetParameters.atmosphereSubdivisions);
        reverseFaces(atmosphereMesh);

        Mesh sphereMesh = generateSphere(planetParameters.baseRadius, planetParameters.planetSubdivisions);

        meshes.push_back(std::move(OpenGLMesh(atmosphereMesh, glm::mat4(1.0))));
        meshes.push_back(std::move(OpenGLMesh(sphereMesh, glm::mat4(1.0))));

        shaderPrograms.push_back(
            createVertexFragmentShaderProgram(
                loadShader(GL_VERTEX_SHADER, "../shaders/AtmosphericScattering.vertex.glsl"),
                loadShader(GL_FRAGMENT_SHADER, "../shaders/AtmosphericScattering.fragment.glsl")));
        shaderPrograms.push_back(
            createVertexFragmentShaderProgram(
                loadShader(GL_VERTEX_SHADER, "../shaders/TerrainGenerator.vertex.glsl"),
                loadShader(GL_FRAGMENT_SHADER, "../shaders/TerrainGenerator.fragment.glsl")));

        textures.push_back(std::move(Texture("../textures/beachMountain.png")));
        textures.push_back(std::move(Texture("../textures/ice.png")));
        textures.push_back(std::move(Texture("../textures/tropic.png")));
        textures.push_back(std::move(Texture("../textures/volcano.png")));

        objects = {
            RenderObject{
                .meshIndex = 0,
                .shaderIndices = std::vector<unsigned int>{0},
                .textureIndex = textures[state.textureIndex].id(),
            },
            RenderObject{
                .meshIndex = 1,
                .shaderIndices = std::vector<unsigned int>{1},
                .textureIndex = textures[state.textureIndex].id()}};

        lightDirection = camera.position();
    }

    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;

    Scene(Scene &&) = default;
    Scene &operator=(Scene &&other) = default;
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
        scene.lightDirection = scene.camera.position();
    }

    scene.state.lastTime = currentTime;
}

void render(GLFWwindow *glfwWindow, const Scene &scene)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, scene.state.wireFrameMode ? GL_LINE : GL_FILL);

    int width, height;
    glfwGetWindowSize(glfwWindow, &width, &height);
    float aspectRatio = (float)width / height;
    glm::vec3 cameraPosition = scene.camera.position();
    glm::mat4 viewMatrix = scene.camera.viewMatrix();
    glm::mat4 projectionMatrix = glm::perspective(scene.camera.fieldOfView, aspectRatio, 0.1f, 10000.0f);
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    for (Scene::RenderObject renderObject : scene.objects)
    {
        renderObject.textureIndex = scene.textures[scene.state.textureIndex].id();
        unsigned int meshId = renderObject.meshIndex;
        const OpenGLMesh &mesh = scene.meshes[meshId];

        glm::mat4 modelViewProjectionMatrix = viewProjectionMatrix * mesh.modelMatrix;

        for (unsigned int shaderIndex : renderObject.shaderIndices)
        {
            const ShaderProgram &shaderProgram = scene.shaderPrograms[shaderIndex];

            glUseProgram(shaderProgram.id());

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, renderObject.textureIndex);

            glUniform1i(glGetUniformLocation(shaderProgram.id(), "heightSlopeBasedColorMap"), 0);
            glUniform3f(glGetUniformLocation(shaderProgram.id(), "lightDirectionInWorldSpace"), scene.lightDirection.x, scene.lightDirection.y, scene.lightDirection.z);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "modelViewProjectionMatrix"), 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "modelMatrix"), 1, GL_FALSE, &mesh.modelMatrix[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "maxNegativeHeight"), scene.planetParameters.maxDepth);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "maxPositiveHeight"), scene.planetParameters.maxHeight);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "baseRadius"), scene.planetParameters.baseRadius);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "atmosphereRadius"), scene.planetParameters.atmosphereRadius());
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "lightPower"), 40000.0f);
            glUniform3f(glGetUniformLocation(shaderProgram.id(), "noiseOffset"), scene.state.noiseOffset.x, scene.state.noiseOffset.y, scene.state.noiseOffset.z);
            glUniform3f(glGetUniformLocation(shaderProgram.id(), "cameraPositionInWorldSpace"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
            glUniform3f(glGetUniformLocation(shaderProgram.id(), "lightColor"), 1, 1, 1);

            glBindBuffer(GL_ARRAY_BUFFER, mesh.getVertexBuffer().id());

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                0,
                0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.getElementBuffer().id());

            glDrawElements(
                GL_TRIANGLES,
                mesh.getNumberOfElements(),
                GL_UNSIGNED_INT,
                0);
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
