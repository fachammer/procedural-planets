#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>

#include <GL/glew.h>
#include <glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/easing.hpp>

#include "GlResources.hpp"

const glm::vec3 UP(0, 1, 0);
const glm::mat4 IDENTITY(1.0f);

struct Planet
{
    float baseRadius = 90;
    float maxDepth = 30;
    float maxHeight = 25;
    float rotateSpeed = 0.03f;
    unsigned int sphereSubdivisions = 7;
    glm::vec3 noiseOffset = glm::vec3(0, 0, 0);
    int textureIndex = 0;
    float angle = 0;

    unsigned int meshIndex;
    unsigned int shaderIndex;
    glm::mat4 modelMatrix;
};

struct Atmosphere
{
    unsigned int sphereSubdivisions = 4;
    float innerRadius;
    float outerRadius;

    unsigned int meshIndex;
    unsigned int shaderIndex;
    glm::mat4 modelMatrix;
};

struct State
{
    bool isPlanetGenerationBlocked = true;
    float lastTime = 0;
};

struct Camera
{
    glm::vec3 position;
    glm::vec3 up;

    float rotateSpeed = 1.5f;

    float fieldOfView = 45.0;
    float aspectRatio = 1.f;

    glm::mat4 viewMatrix() const
    {
        return glm::lookAt(position, glm::vec3(0), up);
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

struct AnimationParameters
{
    glm::vec3 noiseOffset;
    glm::vec3 lightDirection;
};

struct Animation
{
    bool active = false;
    AnimationParameters source;
    AnimationParameters target;
    float progress;
    float duration;

    AnimationParameters current() const
    {
        const float parameter = glm::sineEaseInOut(glm::clamp(progress, 0.0f, 1.0f));
        return AnimationParameters{
            .noiseOffset = parameter * target.noiseOffset + (1 - parameter) * source.noiseOffset,
            .lightDirection = parameter * target.lightDirection + (1 - parameter) * source.lightDirection,
        };
    }
};

struct Scene
{
    std::vector<GlMesh> meshes;
    std::vector<GlShaderProgram> shaderPrograms;
    std::vector<GlTexture> textures;

    Camera camera;
    DirectionalLight light;

    State state;
    Planet planet;
    Atmosphere atmosphere;
    Animation animation;

    Scene()
    {
        atmosphere.innerRadius = planet.baseRadius;
        atmosphere.outerRadius = planet.baseRadius + 5;
        atmosphere.modelMatrix = IDENTITY;
        planet.modelMatrix = IDENTITY;

        Mesh atmosphereMesh = generateSphere(atmosphere.outerRadius, atmosphere.sphereSubdivisions);
        meshes.push_back(GlMesh(atmosphereMesh.indexed_vertices, atmosphereMesh.indices));
        atmosphere.meshIndex = 0;

        Mesh sphereMesh = generateSphere(planet.baseRadius, planet.sphereSubdivisions);
        meshes.push_back(GlMesh(sphereMesh.indexed_vertices, sphereMesh.indices));
        planet.meshIndex = 1;

        GlShaderProgram atmosphericScattering = createVertexFragmentShaderProgram(
            loadShader(GL_VERTEX_SHADER, "assets/shaders/AtmosphericScattering.vertex.glsl"),
            loadShader(GL_FRAGMENT_SHADER, "assets/shaders/AtmosphericScattering.fragment.glsl"));
        shaderPrograms.push_back(std::move(atmosphericScattering));
        atmosphere.shaderIndex = 0;

        GlShaderProgram terrainGenerator = createVertexFragmentShaderProgram(
            loadShader(GL_VERTEX_SHADER, "assets/shaders/TerrainGenerator.vertex.glsl"),
            loadShader(GL_FRAGMENT_SHADER, "assets/shaders/TerrainGenerator.fragment.glsl"));
        shaderPrograms.push_back(std::move(terrainGenerator));
        planet.shaderIndex = 1;

        textures.push_back(GlTexture("assets/textures/beachMountain.png"));
        textures.push_back(GlTexture("assets/textures/ice.png"));
        textures.push_back(GlTexture("assets/textures/tropic.png"));
        textures.push_back(GlTexture("assets/textures/volcano.png"));
        planet.textureIndex = 0;

        light = {
            .direction = -UP,
            .color = glm::vec3(1, 1, 1),
            .power = 1.f,
        };

        animation = Animation{
            .active = false,
            .progress = 0,
            .source = AnimationParameters{
                .lightDirection = light.direction,
                .noiseOffset = planet.noiseOffset,
            },
            .target = AnimationParameters{
                .lightDirection = light.direction,
                .noiseOffset = planet.noiseOffset,
            },
        };

        camera = Camera{
            .position = glm::vec3(0, 0, -250.0),
            .up = glm::vec3(0, 1, 0),
        };
    }

    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;

    Scene(Scene &&) = default;
    Scene &operator=(Scene &&other) = default;
};

void updateCamera(Camera &camera, GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        glm::mat4 rotationMatrix = glm::rotate(IDENTITY, deltaTime * camera.rotateSpeed, glm::cross(camera.position, camera.up));
        camera.position = rotationMatrix * glm::vec4(camera.position, 0);
        camera.up = rotationMatrix * glm::vec4(camera.up, 0);
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        glm::mat4 rotationMatrix = glm::rotate(IDENTITY, -deltaTime * camera.rotateSpeed, glm::cross(camera.position, camera.up));
        camera.position = rotationMatrix * glm::vec4(camera.position, 0);
        camera.up = rotationMatrix * glm::vec4(camera.up, 0);
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        glm::mat4 rotationMatrix = glm::rotate(IDENTITY, deltaTime * camera.rotateSpeed, camera.up);
        camera.position = rotationMatrix * glm::vec4(camera.position, 0);
        camera.up = rotationMatrix * glm::vec4(camera.up, 0);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        glm::mat4 rotationMatrix = glm::rotate(IDENTITY, -deltaTime * camera.rotateSpeed, camera.up);
        camera.position = rotationMatrix * glm::vec4(camera.position, 0);
        camera.up = rotationMatrix * glm::vec4(camera.up, 0);
    }

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    camera.aspectRatio = (float)width / height;
}

void updatePlanetMovement(Scene &scene, float deltaTime)
{
    scene.planet.angle += scene.planet.rotateSpeed * deltaTime;
    scene.planet.modelMatrix = glm::rotate(IDENTITY, scene.planet.angle, UP);
}

float random_in_unit_interval()
{
    return static_cast<float>(arc4random()) / static_cast<float>(RAND_MAX);
}

float random_in_range(float min, float max)
{
    return min + random_in_unit_interval() * (max - min);
}

void updateAnimation(Scene &scene, float deltaTime)
{
    if (!scene.animation.active)
    {
        return;
    }

    scene.animation.progress += deltaTime / scene.animation.duration;
    if (scene.animation.progress >= 1)
    {
        scene.animation.active = false;
    }
    const AnimationParameters parameters = scene.animation.current();
    scene.planet.noiseOffset = parameters.noiseOffset;
    scene.light.direction = parameters.lightDirection;
}

glm::vec3 orthogonal(const glm::vec3 vector)
{
    if (vector.x != 0 || vector.y != 0)
    {
        return glm::vec3(-vector.y, vector.x, 0);
    }
    else if (vector.x != 0 || vector.z != 0)
    {
        return glm::vec3(-vector.z, 0, vector.x);
    }
    else if (vector.y != 0 || vector.z != 0)
    {
        return glm::vec3(0, -vector.z, vector.y);
    }
    else
    {
        throw -1;
    }
}

glm::vec3 random_orthogonal_direction(const glm::vec3 vector)
{
    float phi = random_in_range(0, 3.14);
    glm::vec3 normal = orthogonal(vector);
    glm::vec3 binormal = glm::cross(vector, normal);
    return glm::cos(phi) * normal + glm::sin(phi) * binormal;
}

void update(GLFWwindow *window, Scene &scene)
{
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - scene.state.lastTime);

    updateCamera(scene.camera, window, deltaTime);

    int newNoiseOffset = glfwGetKey(window, GLFW_KEY_SPACE);
    if (newNoiseOffset == GLFW_PRESS && !scene.state.isPlanetGenerationBlocked)
    {
        scene.animation.source = AnimationParameters{
            .noiseOffset = scene.planet.noiseOffset,
            .lightDirection = scene.light.direction,
        };
        float phi = random_in_range(0, 3.14);
        float theta = random_in_range(-1.57, 1.57);
        glm::vec3 rotation_axis = random_orthogonal_direction(scene.light.direction);

        scene.animation.target = AnimationParameters{
            .noiseOffset = scene.planet.noiseOffset + glm::vec3(0.5 * glm::sin(theta) * glm::cos(phi), 0.5 * glm::sin(theta) * glm::sin(phi), 0.5 * glm::cos(phi)),
            .lightDirection = glm::rotate(IDENTITY, 1.0f, rotation_axis) * glm::vec4(scene.light.direction, 0),
        };
        scene.animation.progress = 0;
        scene.animation.duration = 0.5;
        scene.animation.active = true;

        scene.planet.textureIndex = arc4random() % scene.textures.size();
        scene.state.isPlanetGenerationBlocked = true;
    }
    else if (newNoiseOffset == GLFW_RELEASE)
    {
        scene.state.isPlanetGenerationBlocked = false;
    }

    updatePlanetMovement(scene, deltaTime);
    updateAnimation(scene, deltaTime);

    scene.state.lastTime = currentTime;
}

void renderAtmosphere(const Scene &scene)
{
    const glm::vec3 cameraPosition = scene.camera.position;
    const glm::mat4 viewMatrix = scene.camera.viewMatrix();
    const glm::mat4 projectionMatrix = scene.camera.projectionMatrix();
    const glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    const GlMesh &mesh = scene.meshes[scene.atmosphere.meshIndex];
    const glm::mat4 &modelViewProjectionMatrix = viewProjectionMatrix;

    const GlShaderProgram &shaderProgra = scene.shaderPrograms[scene.atmosphere.shaderIndex];
    glUseProgram(shaderProgra.id());

    glUniform3f(glGetUniformLocation(shaderProgra.id(), "lightDirectionInWorldSpace"), scene.light.direction.x, scene.light.direction.y, scene.light.direction.z);
    glUniform1f(glGetUniformLocation(shaderProgra.id(), "lightPower"), scene.light.power);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgra.id(), "modelViewProjectionMatrix"), 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgra.id(), "modelMatrix"), 1, GL_FALSE, &scene.atmosphere.modelMatrix[0][0]);

    glUniform3f(glGetUniformLocation(shaderProgra.id(), "cameraPositionInWorldSpace"), cameraPosition.x, cameraPosition.y, cameraPosition.z);

    glUniform1f(glGetUniformLocation(shaderProgra.id(), "baseRadius"), scene.atmosphere.innerRadius);
    glUniform1f(glGetUniformLocation(shaderProgra.id(), "atmosphereRadius"), scene.atmosphere.outerRadius);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.getVertexBuffer().id());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.getElementBuffer().id());
    glDrawElements(GL_TRIANGLES, mesh.getNumberOfElements(), GL_UNSIGNED_INT, 0);
}

void renderPlanet(const Scene &scene)
{
    const glm::vec3 cameraPosition = scene.camera.position;
    const glm::mat4 viewMatrix = scene.camera.viewMatrix();
    const glm::mat4 projectionMatrix = scene.camera.projectionMatrix();
    const glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    const GlMesh &mesh = scene.meshes[scene.planet.meshIndex];
    const glm::mat4 &modelViewProjectionMatrix = viewProjectionMatrix * scene.planet.modelMatrix;

    const GlShaderProgram &shaderProgram = scene.shaderPrograms[scene.planet.shaderIndex];
    glUseProgram(shaderProgram.id());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene.textures[scene.planet.textureIndex].id());

    glUniform1i(glGetUniformLocation(shaderProgram.id(), "heightSlopeBasedColorMap"), 0);

    glUniform3f(glGetUniformLocation(shaderProgram.id(), "lightDirectionInWorldSpace"), scene.light.direction.x, scene.light.direction.y, scene.light.direction.z);
    glUniform3f(glGetUniformLocation(shaderProgram.id(), "lightColor"), scene.light.color.r, scene.light.color.g, scene.light.color.b);
    glUniform1f(glGetUniformLocation(shaderProgram.id(), "lightPower"), scene.light.power);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "modelViewProjectionMatrix"), 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "modelMatrix"), 1, GL_FALSE, &scene.planet.modelMatrix[0][0]);

    glUniform3f(glGetUniformLocation(shaderProgram.id(), "cameraPositionInWorldSpace"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram.id(), "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);

    glUniform1f(glGetUniformLocation(shaderProgram.id(), "maxNegativeHeight"), scene.planet.maxDepth);
    glUniform1f(glGetUniformLocation(shaderProgram.id(), "maxPositiveHeight"), scene.planet.maxHeight);
    glUniform1f(glGetUniformLocation(shaderProgram.id(), "baseRadius"), scene.planet.baseRadius);
    glUniform3f(glGetUniformLocation(shaderProgram.id(), "noiseOffset"), scene.planet.noiseOffset.x, scene.planet.noiseOffset.y, scene.planet.noiseOffset.z);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.getVertexBuffer().id());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.getElementBuffer().id());
    glDrawElements(GL_TRIANGLES, mesh.getNumberOfElements(), GL_UNSIGNED_INT, 0);
}

void render(GLFWwindow *glfwWindow, const Scene &scene)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisable(GL_DEPTH_TEST);
    renderAtmosphere(scene);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    renderPlanet(scene);

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
