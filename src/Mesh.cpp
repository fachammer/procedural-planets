#include "Mesh.hpp"

#include "GLError.h"

// Include GLEW
#include <GL/glew.h>

#include <map>

// Include GLFW
#include <glfw3.h>

VertexBuffer::VertexBuffer(const std::vector<glm::vec3> &vertices)
{
    bufferId = new unsigned int;
    glGenBuffers(1, bufferId);
    glBindBuffer(GL_ARRAY_BUFFER, *bufferId);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    check_gl_error();
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, bufferId);
    delete bufferId;
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, *bufferId);
}

ElementBuffer::ElementBuffer(const std::vector<unsigned int> &indices)
{
    bufferId = new unsigned int;
    glGenBuffers(1, bufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *bufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    check_gl_error();
}

ElementBuffer::~ElementBuffer()
{
    glDeleteBuffers(1, bufferId);
    delete bufferId;
}

void ElementBuffer::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *bufferId);
}

OpenGLMesh::OpenGLMesh(const Mesh &_mesh, glm::mat4 modelMatrix) : vertexBuffer(_mesh.indexed_vertices), elementBuffer(_mesh.indices), numberOfElements(_mesh.indices.size()), modelMatrix(glm::mat4(1.0))
{
}

void OpenGLMesh::draw() const
{
    glEnableVertexAttribArray(0);

    vertexBuffer.bind();

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void *)0);

    elementBuffer.bind();

    glDrawElements(
        GL_TRIANGLES,
        numberOfElements,
        GL_UNSIGNED_INT,
        (void *)0);
    check_gl_error();
}
