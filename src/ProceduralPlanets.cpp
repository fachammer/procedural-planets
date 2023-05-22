// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>
#include <memory>

// Include GLEW
#include <GL/glew.h>

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
    float rotateSpeed = 0.03f;
    unsigned int atmosphereSubdivisions = 4;
    unsigned int planetSubdivisions = 7;

    float atmosphereRadius() const
    {
        return atmospherePlanetRatio * (baseRadius + maxHeight);
    }
};

struct State
{
    glm::vec3 noiseOffset = glm::vec3(0, 0, 0);
    bool isPlanetGenerationBlocked = true;
    int textureIndex = 0;
    float lastTime = 0;
};

const glm::vec3 UP(0, 1, 0);
struct Camera
{
    float distanceFromOrigin = 250;
    float azimuthalAngle = 0;
    float polarAngle = 0;

    float defaultSpeed = 0.75f;
    float speed = 300.f;
    float rotateSpeed = 1.5f;
    float defaultRotateSpeed = 0.003f;

    float fieldOfView = 45.0;
    float aspectRatio = 1.f;

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

    glm::mat4 projectionMatrix() const
    {
        return glm::perspective(fieldOfView, aspectRatio, 0.1f, 10000.0f);
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

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 color;
    float power;
};

struct Scene
{
    struct RenderObject
    {
        unsigned int meshIndex;
        std::vector<unsigned int> shaderIndices;
    };
    std::vector<RenderObject> objects;
    std::vector<OpenGLMesh> meshes;
    std::vector<ShaderProgram> shaderPrograms;
    std::vector<Texture> textures;

    Camera camera;
    DirectionalLight light;

    State state;
    PlanetParameters planetParameters;

    Scene()
    {
        Mesh atmosphereMesh = generateSphere(planetParameters.atmosphereRadius(), planetParameters.atmosphereSubdivisions);
        reverseFaces(atmosphereMesh);

        Mesh sphereMesh = generateSphere(planetParameters.baseRadius, planetParameters.planetSubdivisions);

        meshes.push_back(OpenGLMesh(atmosphereMesh, glm::mat4(1.0)));
        meshes.push_back(OpenGLMesh(sphereMesh, glm::mat4(1.0)));

        shaderPrograms.push_back(
            createVertexFragmentShaderProgram(
                loadShader(GL_VERTEX_SHADER, "../shaders/AtmosphericScattering.vertex.glsl"),
                loadShader(GL_FRAGMENT_SHADER, "../shaders/AtmosphericScattering.fragment.glsl")));
        shaderPrograms.push_back(
            createVertexFragmentShaderProgram(
                loadShader(GL_VERTEX_SHADER, "../shaders/TerrainGenerator.vertex.glsl"),
                loadShader(GL_FRAGMENT_SHADER, "../shaders/TerrainGenerator.fragment.glsl")));

        textures.push_back(Texture("../textures/beachMountain.png"));
        textures.push_back(Texture("../textures/ice.png"));
        textures.push_back(Texture("../textures/tropic.png"));
        textures.push_back(Texture("../textures/volcano.png"));

        objects = {
            RenderObject{
                .meshIndex = 0,
                .shaderIndices = std::vector<unsigned int>{0},
            },
            RenderObject{
                .meshIndex = 1,
                .shaderIndices = std::vector<unsigned int>{1},
            },
        };

        light = {
            .direction = camera.position(),
            .power = 1.f,
            .color = glm::vec3(1, 1, 1),
        };
    }

    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;

    Scene(Scene &&) = default;
    Scene &operator=(Scene &&other) = default;
};

void updateCamera(Camera &camera, GLFWwindow *window, float deltaTime)
{
    camera.rotateSpeed = camera.defaultRotateSpeed * camera.distanceFromOrigin;
    camera.speed = camera.defaultSpeed * camera.distanceFromOrigin;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.polarAngle += deltaTime * camera.rotateSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.polarAngle -= deltaTime * camera.rotateSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.azimuthalAngle -= deltaTime * camera.rotateSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.azimuthalAngle += deltaTime * camera.rotateSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        camera.distanceFromOrigin -= camera.speed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        camera.distanceFromOrigin += camera.speed * deltaTime;
    }

    camera.polarAngle = glm::clamp(camera.polarAngle, -1.57f, 1.57f);
    camera.distanceFromOrigin = glm::clamp(camera.distanceFromOrigin, 10.0f, 10000.0f);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    camera.aspectRatio = (float)width / height;
}

void updatePlanetMovement(Scene &scene, float deltaTime)
{
    scene.meshes[1].modelMatrix = glm::rotate(scene.meshes[1].modelMatrix, scene.planetParameters.rotateSpeed * deltaTime, UP);
}

float random_in_unit_interval()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float random_in_range(float min, float max)
{
    return min + random_in_unit_interval() * (max - min);
}

void update(GLFWwindow *window, Scene &scene)
{
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - scene.state.lastTime);

    updateCamera(scene.camera, window, deltaTime);

    int newNoiseOffset = glfwGetKey(window, GLFW_KEY_R);
    if (newNoiseOffset == GLFW_PRESS && !scene.state.isPlanetGenerationBlocked)
    {
        scene.state.noiseOffset = glm::vec3(rand() % 99, rand() % 99, rand() % 99);
        scene.state.textureIndex = rand() % scene.textures.size();
        scene.light.direction = glm::vec3(random_in_range(-1, 1), random_in_range(-1, 1), random_in_range(-1, 1));
        scene.state.isPlanetGenerationBlocked = true;
    }
    else if (newNoiseOffset == GLFW_RELEASE)
    {
        scene.state.isPlanetGenerationBlocked = false;
    }

    updatePlanetMovement(scene, deltaTime);

    scene.state.lastTime = currentTime;
}

void render(GLFWwindow *glfwWindow, const Scene &scene)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glm::vec3 cameraPosition = scene.camera.position();
    glm::mat4 viewMatrix = scene.camera.viewMatrix();
    glm::mat4 projectionMatrix = scene.camera.projectionMatrix();
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    for (Scene::RenderObject renderObject : scene.objects)
    {
        const OpenGLMesh &mesh = scene.meshes[renderObject.meshIndex];

        glm::mat4 modelViewProjectionMatrix = viewProjectionMatrix * mesh.modelMatrix;

        for (unsigned int shaderIndex : renderObject.shaderIndices)
        {
            const ShaderProgram &shaderProgram = scene.shaderPrograms[shaderIndex];
            glUseProgram(shaderProgram.id());

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, scene.textures[scene.state.textureIndex].id());

            glUniform1i(glGetUniformLocation(shaderProgram.id(), "heightSlopeBasedColorMap"), 0);

            glUniform3f(glGetUniformLocation(shaderProgram.id(), "lightDirectionInWorldSpace"), scene.light.direction.x, scene.light.direction.y, scene.light.direction.z);
            glUniform3f(glGetUniformLocation(shaderProgram.id(), "lightColor"), scene.light.color.r, scene.light.color.g, scene.light.color.b);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "lightPower"), scene.light.power);

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "modelViewProjectionMatrix"), 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "modelMatrix"), 1, GL_FALSE, &mesh.modelMatrix[0][0]);

            glUniform3f(glGetUniformLocation(shaderProgram.id(), "cameraPositionInWorldSpace"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);

            glUniform1f(glGetUniformLocation(shaderProgram.id(), "maxNegativeHeight"), scene.planetParameters.maxDepth);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "maxPositiveHeight"), scene.planetParameters.maxHeight);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "baseRadius"), scene.planetParameters.baseRadius);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "atmosphereRadius"), scene.planetParameters.atmosphereRadius());
            glUniform3f(glGetUniformLocation(shaderProgram.id(), "noiseOffset"), scene.state.noiseOffset.x, scene.state.noiseOffset.y, scene.state.noiseOffset.z);

            glBindBuffer(GL_ARRAY_BUFFER, mesh.getVertexBuffer().id());
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.getElementBuffer().id());
            glDrawElements(GL_TRIANGLES, mesh.getNumberOfElements(), GL_UNSIGNED_INT, 0);
        }
    }

    glfwSwapBuffers(glfwWindow);
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
    Window(unsigned int initialWidth, unsigned int initialHeight)
    {
        glfwWindowHint(GLFW_SAMPLES, 8);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(initialWidth, initialHeight, "Procedural Planets", NULL, NULL);
        if (window == NULL)
        {
            throw -1;
        }
        glfwMakeContextCurrent(window);

        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        glfwSetCursorPos(window, initialWidth / 2, initialHeight / 2);
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
            Window window(1024, 1024);
            try
            {
                Glew glew;
                Scene scene;
                GLFWwindow *glfwWindow = window.glfwWindow();
                BoundVertexArrayObject vao;
                do
                {
                    glfwPollEvents();
                    update(glfwWindow, scene);
                    render(glfwWindow, scene);
                } while (!glfwWindowShouldClose(glfwWindow));
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
