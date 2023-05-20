#include "Mesh.hpp"

#include "GLError.h"

// Include GLEW
#include <GL/glew.h>

#include <map>

// Include GLFW
#include <glfw3.h>

OpenGLMesh::OpenGLMesh(Mesh _mesh, glm::mat4 _modelMatrix) : mesh(_mesh), modelMatrix(_modelMatrix)
{
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mesh.indexed_vertices.size() * sizeof(glm::vec3), &mesh.indexed_vertices[0], GL_STATIC_DRAW);
    check_gl_error();

    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);
    check_gl_error();
}

OpenGLMesh::~OpenGLMesh()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &elementBuffer);
}

void OpenGLMesh::draw()
{
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    glVertexAttribPointer(
        0,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );
    check_gl_error();

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

    check_gl_error();
    // Draw the triangles !
    glDrawElements(
        GL_TRIANGLES,        // mode
        mesh.indices.size(), // count
        GL_UNSIGNED_INT,     // type
        (void *)0            // element array buffer offset
    );
    check_gl_error();
}
