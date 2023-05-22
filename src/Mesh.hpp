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
    unsigned int bufferId;

public:
    VertexBuffer() : bufferId(0) {}
    VertexBuffer(const std::vector<glm::vec3> &vertices);
    ~VertexBuffer();

    VertexBuffer(VertexBuffer &buffer) = delete;
    VertexBuffer operator=(VertexBuffer &buffer) = delete;

    VertexBuffer(VertexBuffer &&buffer) : bufferId(buffer.bufferId)
    {
        buffer.bufferId = 0;
    }

    VertexBuffer &operator=(VertexBuffer &&buffer)
    {
        if (this != &buffer)
        {
            bufferId = buffer.bufferId;
            buffer.bufferId = 0;
        }
        return *this;
    }

    void bind() const;
};

class ElementBuffer
{
private:
    unsigned int bufferId;

public:
    ElementBuffer() : bufferId(0) {}
    ElementBuffer(const std::vector<unsigned int> &indices);
    ~ElementBuffer();

    ElementBuffer(ElementBuffer &buffer) = delete;
    ElementBuffer operator=(ElementBuffer &buffer) = delete;

    ElementBuffer(ElementBuffer &&buffer) : bufferId(buffer.bufferId)
    {
        buffer.bufferId = 0;
    }

    ElementBuffer &operator=(ElementBuffer &&buffer)
    {
        if (this != &buffer)
        {
            bufferId = buffer.bufferId;
            buffer.bufferId = 0;
        }
        return *this;
    }

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

    OpenGLMesh() : numberOfElements(0) {}
    OpenGLMesh(const Mesh &_mesh, glm::mat4 _modelMatrix);

    OpenGLMesh(const OpenGLMesh &) = delete;
    OpenGLMesh &operator=(const OpenGLMesh &) = delete;

    OpenGLMesh(OpenGLMesh &&mesh)
        : vertexBuffer(std::move(mesh.vertexBuffer)),
          elementBuffer(std::move(mesh.elementBuffer)),
          numberOfElements(mesh.numberOfElements)
    {
        mesh.numberOfElements = 0;
    }

    OpenGLMesh &operator=(OpenGLMesh &&mesh)
    {
        if (this != &mesh)
        {
            vertexBuffer = std::move(mesh.vertexBuffer);
            elementBuffer = std::move(mesh.elementBuffer);
            numberOfElements = mesh.numberOfElements;
            mesh.numberOfElements = 0;
        }
        return *this;
    }

    void draw() const;
};
