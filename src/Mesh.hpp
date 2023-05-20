#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "GLError.h"

class ShaderEffect;

struct Mesh
{
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> indexed_vertices;
};

class OpenGLMesh
{
private:
    Mesh mesh;
    unsigned int vertexBuffer;
    unsigned int elementBuffer;

public:
    glm::mat4 modelMatrix;

public:
    OpenGLMesh(Mesh _mesh, glm::mat4 _modelMatrix);
    ~OpenGLMesh();

    void draw();
};
