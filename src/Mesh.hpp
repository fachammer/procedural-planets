#pragma once

#include <vector>
#include <glm/glm.hpp>

class ShaderEffect;

class Mesh
{
public:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> biTangents;

    std::vector<unsigned int> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    std::vector<glm::vec3> indexed_tangents;
    std::vector<glm::vec3> indexed_biTangents;

    glm::mat4 modelMatrix;

    unsigned int vertexBuffer;
    unsigned int uvBuffer;
    unsigned int normalBuffer;
    unsigned int tangentBuffer;
    unsigned int biTangentBuffer;
    unsigned int elementBuffer;

    ShaderEffect *shaderProgram;
    std::vector<unsigned int> textureIds;

    bool load(const char *path);
    // only works for non-indexed meshes
    void calculateTangents();
    void generateVBOs();
    void bindBuffersAndDraw();
    void createQuad(glm::vec2 ll, glm::vec2 ur);
    void createCube(glm::vec3 dimensions, bool frontFacing);

    ~Mesh();
    Mesh();
};
