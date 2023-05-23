#include "Mesh.hpp"
#include <GL/glew.h>
#include <glfw3.h>

GlVertexBuffer::GlVertexBuffer(const std::vector<glm::vec3> &vertices)
{
    glGenBuffers(1, &bufferId);
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
}

GlVertexBuffer::~GlVertexBuffer()
{
    glDeleteBuffers(1, &bufferId);
}

GlElementBuffer::GlElementBuffer(const std::vector<unsigned int> &indices)
{
    glGenBuffers(1, &bufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

GlElementBuffer::~GlElementBuffer()
{
    glDeleteBuffers(1, &bufferId);
}

GlMesh::GlMesh(const Mesh &_mesh, glm::mat4 modelMatrix)
    : vertexBuffer(_mesh.indexed_vertices),
      elementBuffer(_mesh.indices),
      numberOfElements(_mesh.indices.size()),
      modelMatrix(glm::mat4(1.0))
{
}
