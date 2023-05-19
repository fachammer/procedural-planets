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

    void generateVBOs();
    void bindBuffersAndDraw();
    void reverseFaces();

    ~Mesh();
    Mesh();
};
