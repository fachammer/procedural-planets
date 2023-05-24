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

const glm::vec3 UP(0, 1, 0);
const glm::mat4 IDENTITY(1.0f);

struct Planet
{
    float baseRadius = 90;
    float maxDepth = 30;
    float maxHeight = 25;
    float atmospherePlanetRatio = 1.12;
    float rotateSpeed = 0.03f;
    unsigned int sphereSubdivisions = 7;
    glm::vec3 noiseOffset = glm::vec3(0, 0, 0);
    int textureIndex = 0;
    float angle = 0;

    unsigned int meshIndex;
    unsigned int shaderIndex;
    glm::mat4 modelMatrix;

    float atmosphereRadius() const
    {
        return atmospherePlanetRatio * baseRadius;
    }
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

struct AtmosphericScatteringUniformLocations
{
    GLint lightDirectionInWorldSpace;
    GLint lightPower;
    GLint modelViewProjectionMatrix;
    GLint modelMatrix;
    GLint cameraPositionInWorldSpace;
    GLint baseRadius;
    GLint atmosphereRadius;

    AtmosphericScatteringUniformLocations() {}

    AtmosphericScatteringUniformLocations(const GlShaderProgram &program)
    {
        lightDirectionInWorldSpace = glGetUniformLocation(program.id(), "lightDirectionInWorldSpace");
        lightPower = glGetUniformLocation(program.id(), "lightPower");
        modelViewProjectionMatrix = glGetUniformLocation(program.id(), "modelViewProjectionMatrix");
        modelMatrix = glGetUniformLocation(program.id(), "modelMatrix");
        cameraPositionInWorldSpace = glGetUniformLocation(program.id(), "cameraPositionInWorldSpace");
        baseRadius = glGetUniformLocation(program.id(), "baseRadius");
        atmosphereRadius = glGetUniformLocation(program.id(), "atmosphereRadius");
    }
};

struct TerrainGeneratorUniformLocations
{
    GLint heightSlopeBasedColorMap;
    GLint lightDirectionInWorldSpace;
    GLint lightColor;
    GLint lightPower;
    GLint modelViewProjectionMatrix;
    GLint modelMatrix;
    GLint cameraPositionInWorldSpace;
    GLint viewMatrix;
    GLint maxNegativeHeight;
    GLint maxPositiveHeight;
    GLint baseRadius;
    GLint noiseOffset;

    TerrainGeneratorUniformLocations() {}

    TerrainGeneratorUniformLocations(const GlShaderProgram &program)
    {
        heightSlopeBasedColorMap = glGetUniformLocation(program.id(), "heightSlopeBasedColorMap");
        lightDirectionInWorldSpace = glGetUniformLocation(program.id(), "lightDirectionInWorldSpace");
        lightColor = glGetUniformLocation(program.id(), "lightColor");
        lightPower = glGetUniformLocation(program.id(), "lightPower");
        modelViewProjectionMatrix = glGetUniformLocation(program.id(), "modelViewProjectionMatrix");
        modelMatrix = glGetUniformLocation(program.id(), "modelMatrix");
        cameraPositionInWorldSpace = glGetUniformLocation(program.id(), "cameraPositionInWorldSpace");
        viewMatrix = glGetUniformLocation(program.id(), "viewMatrix");
        maxNegativeHeight = glGetUniformLocation(program.id(), "maxNegativeHeight");
        maxPositiveHeight = glGetUniformLocation(program.id(), "maxPositiveHeight");
        baseRadius = glGetUniformLocation(program.id(), "baseRadius");
        noiseOffset = glGetUniformLocation(program.id(), "noiseOffset");
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
    TerrainGeneratorUniformLocations terrainGeneratorUniformLocations;
    AtmosphericScatteringUniformLocations atmosphericScatteringUniformLocations;

    Scene()
    {
        atmosphere.innerRadius = planet.baseRadius;
        atmosphere.outerRadius = planet.baseRadius * planet.atmospherePlanetRatio;
        atmosphere.modelMatrix = IDENTITY;
        planet.modelMatrix = IDENTITY;

        Mesh atmosphereMesh = generateSphere(atmosphere.outerRadius, atmosphere.sphereSubdivisions);
        reverseFaces(atmosphereMesh);
        meshes.push_back(GlMesh(atmosphereMesh.indexed_vertices, atmosphereMesh.indices));
        atmosphere.meshIndex = 0;

        Mesh sphereMesh = generateSphere(planet.baseRadius, planet.sphereSubdivisions);
        meshes.push_back(GlMesh(sphereMesh.indexed_vertices, sphereMesh.indices));
        planet.meshIndex = 1;

        GlShaderProgram atmosphericScattering = createVertexFragmentShaderProgram(
            loadShader(GL_VERTEX_SHADER, "../shaders/AtmosphericScattering.vertex.glsl"),
            loadShader(GL_FRAGMENT_SHADER, "../shaders/AtmosphericScattering.fragment.glsl"));
        atmosphericScatteringUniformLocations = AtmosphericScatteringUniformLocations(atmosphericScattering);
        shaderPrograms.push_back(std::move(atmosphericScattering));
        atmosphere.shaderIndex = 0;

        GlShaderProgram terrainGenerator = createVertexFragmentShaderProgram(
            loadShader(GL_VERTEX_SHADER, "../shaders/TerrainGenerator.vertex.glsl"),
            loadShader(GL_FRAGMENT_SHADER, "../shaders/TerrainGenerator.fragment.glsl"));
        terrainGeneratorUniformLocations = TerrainGeneratorUniformLocations(terrainGenerator);
        shaderPrograms.push_back(std::move(terrainGenerator));
        planet.shaderIndex = 1;

        textures.push_back(GlTexture("../textures/beachMountain.png"));
        textures.push_back(GlTexture("../textures/ice.png"));
        textures.push_back(GlTexture("../textures/tropic.png"));
        textures.push_back(GlTexture("../textures/volcano.png"));
        planet.textureIndex = 0;

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

void update(GLFWwindow *window, Scene &scene)
{
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - scene.state.lastTime);

    updateCamera(scene.camera, window, deltaTime);

    int newNoiseOffset = glfwGetKey(window, GLFW_KEY_R);
    if (newNoiseOffset == GLFW_PRESS && !scene.state.isPlanetGenerationBlocked)
    {
        scene.planet.noiseOffset = glm::vec3(rand() % 99, rand() % 99, rand() % 99);
        scene.planet.textureIndex = arc4random() % scene.textures.size();
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

void renderAtmosphere(const Scene &scene)
{
    const glm::vec3 cameraPosition = scene.camera.position();
    const glm::mat4 viewMatrix = scene.camera.viewMatrix();
    const glm::mat4 projectionMatrix = scene.camera.projectionMatrix();
    const glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    const GlMesh &mesh = scene.meshes[scene.atmosphere.meshIndex];
    const glm::mat4 &modelViewProjectionMatrix = viewProjectionMatrix;

    const GlShaderProgram &shaderProgram = scene.shaderPrograms[scene.atmosphere.shaderIndex];
    glUseProgram(shaderProgram.id());
    const AtmosphericScatteringUniformLocations &uniformLocations = scene.atmosphericScatteringUniformLocations;

    glUniform3f(uniformLocations.lightDirectionInWorldSpace, scene.light.direction.x, scene.light.direction.y, scene.light.direction.z);
    glUniform1f(uniformLocations.lightPower, scene.light.power);

    glUniformMatrix4fv(uniformLocations.modelViewProjectionMatrix, 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);
    glUniformMatrix4fv(uniformLocations.modelMatrix, 1, GL_FALSE, &scene.atmosphere.modelMatrix[0][0]);

    glUniform3f(uniformLocations.cameraPositionInWorldSpace, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    glUniform1f(uniformLocations.baseRadius, scene.atmosphere.innerRadius);
    glUniform1f(uniformLocations.atmosphereRadius, scene.atmosphere.outerRadius);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.getVertexBuffer().id());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.getElementBuffer().id());
    glDrawElements(GL_TRIANGLES, mesh.getNumberOfElements(), GL_UNSIGNED_INT, 0);
}

void renderPlanet(const Scene &scene)
{
    const glm::vec3 cameraPosition = scene.camera.position();
    const glm::mat4 viewMatrix = scene.camera.viewMatrix();
    const glm::mat4 projectionMatrix = scene.camera.projectionMatrix();
    const glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    const GlMesh &mesh = scene.meshes[scene.planet.meshIndex];
    const glm::mat4 &modelViewProjectionMatrix = viewProjectionMatrix * scene.planet.modelMatrix;

    const GlShaderProgram &shaderProgram = scene.shaderPrograms[scene.planet.shaderIndex];
    glUseProgram(shaderProgram.id());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene.textures[scene.planet.textureIndex].id());
    const TerrainGeneratorUniformLocations &uniformLocations = scene.terrainGeneratorUniformLocations;

    glUniform1i(uniformLocations.heightSlopeBasedColorMap, 0);

    glUniform3f(uniformLocations.lightDirectionInWorldSpace, scene.light.direction.x, scene.light.direction.y, scene.light.direction.z);
    glUniform3f(uniformLocations.lightColor, scene.light.color.r, scene.light.color.g, scene.light.color.b);
    glUniform1f(uniformLocations.lightPower, scene.light.power);

    glUniformMatrix4fv(uniformLocations.modelViewProjectionMatrix, 1, GL_FALSE, &modelViewProjectionMatrix[0][0]);
    glUniformMatrix4fv(uniformLocations.modelMatrix, 1, GL_FALSE, &scene.planet.modelMatrix[0][0]);

    glUniform3f(uniformLocations.cameraPositionInWorldSpace, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    glUniformMatrix4fv(uniformLocations.viewMatrix, 1, GL_FALSE, &viewMatrix[0][0]);

    glUniform1f(uniformLocations.maxNegativeHeight, scene.planet.maxDepth);
    glUniform1f(uniformLocations.maxPositiveHeight, scene.planet.maxHeight);
    glUniform1f(uniformLocations.baseRadius, scene.planet.baseRadius);
    glUniform3f(uniformLocations.noiseOffset, scene.planet.noiseOffset.x, scene.planet.noiseOffset.y, scene.planet.noiseOffset.z);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.getVertexBuffer().id());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.getElementBuffer().id());
    glDrawElements(GL_TRIANGLES, mesh.getNumberOfElements(), GL_UNSIGNED_INT, 0);
}

void render(GLFWwindow *glfwWindow, const Scene &scene)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    renderAtmosphere(scene);
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
