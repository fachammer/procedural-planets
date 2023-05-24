#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>

#include <GL/glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GlResources.hpp"

struct PlanetParameters
{
    float baseRadius = 90;
    float maxDepth = 30;
    float maxHeight = 25;
    float atmospherePlanetRatio = 1.12;
    float rotateSpeed = 0.03f;
    unsigned int atmosphereSubdivisions = 4;
    unsigned int planetSubdivisions = 7;
    glm::vec3 noiseOffset = glm::vec3(0, 0, 0);
    int textureIndex = 0;

    float atmosphereRadius() const
    {
        return atmospherePlanetRatio * (baseRadius);
    }
};

struct State
{
    bool isPlanetGenerationBlocked = true;
    float lastTime = 0;
};

const glm::vec3 UP(0, 1, 0);
struct Camera
{
    float distanceFromOrigin = 250;
    float azimuthalAngle = 0;
    float polarAngle = 0;

    float rotateSpeed = 1.5f;

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

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 color;
    float power;
};

struct Mesh
{
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> indexed_vertices;
};

static const GLdouble theta = 0.5 * (1.0 + sqrt(5.0));
static const glm::vec3 ICOSAHEDRON_VERTICES[12] = {
    glm::vec3(-1, theta, 0),
    glm::vec3(1, theta, 0),
    glm::vec3(-1, -theta, 0),
    glm::vec3(1, -theta, 0),

    glm::vec3(0, -1, theta),
    glm::vec3(0, 1, theta),
    glm::vec3(0, -1, -theta),
    glm::vec3(0, 1, -theta),

    glm::vec3(theta, 0, -1),
    glm::vec3(theta, 0, 1),
    glm::vec3(-theta, 0, -1),
    glm::vec3(-theta, 0, 1)};

static const unsigned int ICOSAHEDRON_INDICES[60] = {
    0, 11, 5,
    0, 5, 1,
    0, 1, 7,
    0, 7, 10,
    0, 10, 11,

    1, 5, 9,
    5, 11, 4,
    11, 10, 2,
    10, 7, 6,
    7, 1, 8,

    3, 9, 4,
    3, 4, 2,
    3, 2, 6,
    3, 6, 8,
    3, 8, 9,

    4, 9, 5,
    2, 4, 11,
    6, 2, 10,
    8, 6, 7,
    9, 8, 1};

static int addSphereVertex(Mesh &mesh, glm::vec3 vertex, GLfloat radius)
{
    glm::vec3 normalizedVertex = glm::normalize(vertex * radius);
    mesh.indexed_vertices.push_back(normalizedVertex * radius);
    return mesh.indexed_vertices.size() - 1;
}

Mesh generateSphere(GLfloat radius, int subdivisions)
{
    Mesh sphere;

    for (int i = 0; i < 12; i++)
    {
        addSphereVertex(sphere, ICOSAHEDRON_VERTICES[i], radius);
    }

    for (int i = 0; i < 60; i++)
    {
        sphere.indices.push_back(ICOSAHEDRON_INDICES[i]);
    }

    for (int s = 0; s < subdivisions; s++)
    {
        std::vector<unsigned int> subdividedSphereIndices;

        for (unsigned int i = 0; (i + 2) < sphere.indices.size(); i += 3)
        {
            int aIndex = sphere.indices[i];
            int bIndex = sphere.indices[i + 1];
            int cIndex = sphere.indices[i + 2];

            glm::vec3 a = sphere.indexed_vertices[aIndex];
            glm::vec3 b = sphere.indexed_vertices[bIndex];
            glm::vec3 c = sphere.indexed_vertices[cIndex];

            glm::vec3 ab = a + b;
            glm::vec3 bc = b + c;
            glm::vec3 ca = c + a;

            int abIndex = addSphereVertex(sphere, ab, radius);
            int bcIndex = addSphereVertex(sphere, bc, radius);
            int caIndex = addSphereVertex(sphere, ca, radius);

            subdividedSphereIndices.push_back(aIndex);
            subdividedSphereIndices.push_back(abIndex);
            subdividedSphereIndices.push_back(caIndex);

            subdividedSphereIndices.push_back(bIndex);
            subdividedSphereIndices.push_back(bcIndex);
            subdividedSphereIndices.push_back(abIndex);

            subdividedSphereIndices.push_back(cIndex);
            subdividedSphereIndices.push_back(caIndex);
            subdividedSphereIndices.push_back(bcIndex);

            subdividedSphereIndices.push_back(abIndex);
            subdividedSphereIndices.push_back(bcIndex);
            subdividedSphereIndices.push_back(caIndex);
        }

        sphere.indices = subdividedSphereIndices;
    }

    return sphere;
}

void reverseFaces(Mesh &mesh)
{
    std::vector<unsigned int> reversedIndices;

    for (unsigned int i = 0; i < mesh.indices.size(); i += 3)
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
        glm::mat4 modelMatrix;
    };
    std::vector<RenderObject> objects;
    std::vector<GlMesh> meshes;
    std::vector<GlShaderProgram> shaderPrograms;
    std::vector<GlTexture> textures;

    Camera camera;
    DirectionalLight light;

    State state;
    PlanetParameters planetParameters;

    Scene()
    {
        Mesh atmosphereMesh = generateSphere(planetParameters.atmosphereRadius(), planetParameters.atmosphereSubdivisions);
        reverseFaces(atmosphereMesh);

        Mesh sphereMesh = generateSphere(planetParameters.baseRadius, planetParameters.planetSubdivisions);

        meshes.push_back(GlMesh(atmosphereMesh.indexed_vertices, atmosphereMesh.indices));
        meshes.push_back(GlMesh(sphereMesh.indexed_vertices, sphereMesh.indices));

        shaderPrograms.push_back(
            createVertexFragmentShaderProgram(
                loadShader(GL_VERTEX_SHADER, "../shaders/AtmosphericScattering.vertex.glsl"),
                loadShader(GL_FRAGMENT_SHADER, "../shaders/AtmosphericScattering.fragment.glsl")));
        shaderPrograms.push_back(
            createVertexFragmentShaderProgram(
                loadShader(GL_VERTEX_SHADER, "../shaders/TerrainGenerator.vertex.glsl"),
                loadShader(GL_FRAGMENT_SHADER, "../shaders/TerrainGenerator.fragment.glsl")));

        textures.push_back(GlTexture("../textures/beachMountain.png"));
        textures.push_back(GlTexture("../textures/ice.png"));
        textures.push_back(GlTexture("../textures/tropic.png"));
        textures.push_back(GlTexture("../textures/volcano.png"));

        objects = {
            RenderObject{
                .meshIndex = 0,
                .shaderIndices = std::vector<unsigned int>{0},
                .modelMatrix = glm::mat4(1.0f),
            },
            RenderObject{
                .meshIndex = 1,
                .shaderIndices = std::vector<unsigned int>{1},
                .modelMatrix = glm::mat4(1.0f),
            },
        };

        light = {
            .direction = -UP,
            .color = glm::vec3(1, 1, 1),
            .power = 1.f,
        };
    }

    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;

    Scene(Scene &&) = default;
    Scene &operator=(Scene &&other) = default;

    RenderObject &planet() { return objects[1]; }
};

void updateCamera(Camera &camera, GLFWwindow *window, float deltaTime)
{
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

    camera.polarAngle = glm::clamp(camera.polarAngle, -1.57f, 1.57f);
    camera.distanceFromOrigin = glm::clamp(camera.distanceFromOrigin, 10.0f, 10000.0f);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    camera.aspectRatio = (float)width / height;
}

void updatePlanetMovement(Scene &scene, float deltaTime)
{
    scene.planet().modelMatrix = glm::rotate(scene.planet().modelMatrix, scene.planetParameters.rotateSpeed * deltaTime, UP);
}

float random_in_unit_interval()
{
    return static_cast<float>(arc4random()) / static_cast<float>(RAND_MAX);
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
        scene.planetParameters.noiseOffset = glm::vec3(rand() % 99, rand() % 99, rand() % 99);
        scene.planetParameters.textureIndex = arc4random() % scene.textures.size();
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
        const GlMesh &mesh = scene.meshes[renderObject.meshIndex];
        glm::mat4 modelViewProjectionMatrix = viewProjectionMatrix * renderObject.modelMatrix;

        for (unsigned int shaderIndex : renderObject.shaderIndices)
        {
            const GlShaderProgram &shaderProgram = scene.shaderPrograms[shaderIndex];
            glUseProgram(shaderProgram.id());

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, scene.textures[scene.planetParameters.textureIndex].id());
            glUniform1i(glGetUniformLocation(shaderProgram.id(), "heightSlopeBasedColorMap"), 0);

            glUniform3f(glGetUniformLocation(shaderProgram.id(), "lightDirectionInWorldSpace"), scene.light.direction.x, scene.light.direction.y, scene.light.direction.z);
            glUniform3f(glGetUniformLocation(shaderProgram.id(), "lightColor"), scene.light.color.r, scene.light.color.g, scene.light.color.b);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "lightPower"), scene.light.power);

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "modelViewProjectionMatrix"), 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "modelMatrix"), 1, GL_FALSE, &renderObject.modelMatrix[0][0]);

            glUniform3f(glGetUniformLocation(shaderProgram.id(), "cameraPositionInWorldSpace"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);

            glUniform1f(glGetUniformLocation(shaderProgram.id(), "maxNegativeHeight"), scene.planetParameters.maxDepth);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "maxPositiveHeight"), scene.planetParameters.maxHeight);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "baseRadius"), scene.planetParameters.baseRadius);
            glUniform1f(glGetUniformLocation(shaderProgram.id(), "atmosphereRadius"), scene.planetParameters.atmosphereRadius());
            glUniform3f(glGetUniformLocation(shaderProgram.id(), "noiseOffset"), scene.planetParameters.noiseOffset.x, scene.planetParameters.noiseOffset.y, scene.planetParameters.noiseOffset.z);

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

int main(void)
{
    try
    {
        Glfw glfw;
        try
        {
            GlfwWindow window(1024, 1024, "Procedural Celestial Bodies");
            try
            {
                Glew glew;
                Scene scene;
                GLFWwindow *glfwWindow = window.glfwWindow();
                GlVertexArrayObject vao;
                glBindVertexArray(vao.id());
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
