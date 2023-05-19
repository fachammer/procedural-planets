#include "Mesh.hpp"

#include "GLError.h"

// Include GLEW
#include <GL/glew.h>

#include <map>

// Include GLFW
#include <glfw3.h>

void Mesh::generateVBOs()
{
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

void Mesh::bindBuffersAndDraw()
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
        GL_TRIANGLES,    // mode
        indices.size(),  // count
        GL_UNSIGNED_INT, // type
        (void *)0        // element array buffer offset
    );
    check_gl_error();
}

void Mesh::reverseFaces()
{
    std::vector<unsigned int> reversedIndices;

    for (int i = 0; i < this->indices.size(); i += 3)
    {
        for (int j = 2; j >= 0; j--)
        {
            reversedIndices.push_back(this->indices[i + j]);
        }
    }

    this->indices = reversedIndices;
}

Mesh::Mesh() : modelMatrix(glm::mat4(1.0))
{
}

Mesh::~Mesh()
{
}
