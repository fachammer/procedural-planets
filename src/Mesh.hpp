#pragma once

#include <vector>
#include <glm/glm.hpp>

class ShaderEffect;

class Mesh
{
public:
    std::vector<glm::vec3> vertices;

    std::vector<unsigned int> indices;
    std::vector<glm::vec3> indexed_vertices;

    glm::mat4 modelMatrix;

    unsigned int vertexBuffer;
    unsigned int elementBuffer;

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
