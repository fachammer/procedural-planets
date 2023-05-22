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

    unsigned int id() const
    {
        return bufferId;
    }
};

class ElementBuffer
{
private:
    unsigned int bufferId;

public:
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

    unsigned int id() const
    {
        return bufferId;
    }
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

    OpenGLMesh(const OpenGLMesh &) = delete;
    OpenGLMesh &operator=(const OpenGLMesh &) = delete;

    OpenGLMesh(OpenGLMesh &&mesh) = default;
    OpenGLMesh &operator=(OpenGLMesh &&mesh) = default;

    const VertexBuffer &getVertexBuffer() const
    {
        return vertexBuffer;
    }

    const ElementBuffer &getElementBuffer() const
    {
        return elementBuffer;
    }

    unsigned int getNumberOfElements() const
    {
        return numberOfElements;
    }
};
