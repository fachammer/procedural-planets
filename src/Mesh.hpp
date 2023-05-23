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

class GlVertexBuffer
{
private:
    unsigned int bufferId;

public:
    GlVertexBuffer(const std::vector<glm::vec3> &vertices);
    ~GlVertexBuffer();

    GlVertexBuffer(GlVertexBuffer &buffer) = delete;
    GlVertexBuffer operator=(GlVertexBuffer &buffer) = delete;

    GlVertexBuffer(GlVertexBuffer &&buffer) : bufferId(buffer.bufferId)
    {
        buffer.bufferId = 0;
    }

    GlVertexBuffer &operator=(GlVertexBuffer &&buffer)
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

class GlElementBuffer
{
private:
    unsigned int bufferId;

public:
    GlElementBuffer(const std::vector<unsigned int> &indices);
    ~GlElementBuffer();

    GlElementBuffer(GlElementBuffer &buffer) = delete;
    GlElementBuffer operator=(GlElementBuffer &buffer) = delete;

    GlElementBuffer(GlElementBuffer &&buffer) : bufferId(buffer.bufferId)
    {
        buffer.bufferId = 0;
    }

    GlElementBuffer &operator=(GlElementBuffer &&buffer)
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

class GlMesh
{
private:
    GlVertexBuffer vertexBuffer;
    GlElementBuffer elementBuffer;
    unsigned int numberOfElements;

public:
    glm::mat4 modelMatrix;

    GlMesh(const Mesh &_mesh, glm::mat4 _modelMatrix);

    GlMesh(const GlMesh &) = delete;
    GlMesh &operator=(const GlMesh &) = delete;

    GlMesh(GlMesh &&mesh) = default;
    GlMesh &operator=(GlMesh &&mesh) = default;

    const GlVertexBuffer &getVertexBuffer() const
    {
        return vertexBuffer;
    }

    const GlElementBuffer &getElementBuffer() const
    {
        return elementBuffer;
    }

    unsigned int getNumberOfElements() const
    {
        return numberOfElements;
    }
};
