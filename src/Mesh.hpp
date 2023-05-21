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

class VertexBuffer
{
private:
    unsigned int *bufferId;

public:
    VertexBuffer(const std::vector<glm::vec3> &vertices);
    ~VertexBuffer();

    void bind() const;
};

class ElementBuffer
{
private:
    unsigned int *bufferId;

public:
    ElementBuffer(const std::vector<unsigned int> &indices);
    ~ElementBuffer();

    void bind() const;
};

class OpenGLMesh
{
private:
    VertexBuffer vertexBuffer;
    ElementBuffer elementBuffer;
    unsigned int numberOfElements;

public:
    glm::mat4 modelMatrix;

    OpenGLMesh(const Mesh &_mesh, glm::mat4 _modelMatrix);

    void draw() const;
};
