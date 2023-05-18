#include "Mesh.hpp"

#include "GLError.h"

// Include GLEW
#include <GL/glew.h>

#include <map>

// Include GLFW
#include <glfw3.h>

struct PackedVertexWithTangents2
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 biTangent;
    bool operator<(const PackedVertexWithTangents2 that) const
    {
        double count = position.x + position.y + position.z + uv.x + uv.y;
        double countThat = that.position.x + that.position.y + that.position.z + that.uv.x + that.uv.y;
        return count < countThat;
    };
};

static const GLfloat quad_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f};
static const GLfloat quad_uv_buffer_data[] = {
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
};
static const GLfloat quad_index_buffer_data[] = {
    0, 1, 2,
    2, 1, 3};

void Mesh::generateVBOs()
{
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

    if (indexed_tangents.size() == indexed_vertices.size())
    {
        glGenBuffers(1, &tangentBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_tangents.size() * sizeof(glm::vec3), &indexed_tangents[0], GL_STATIC_DRAW);

        glGenBuffers(1, &biTangentBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, biTangentBuffer);
        glBufferData(GL_ARRAY_BUFFER, indexed_biTangents.size() * sizeof(glm::vec3), &indexed_biTangents[0], GL_STATIC_DRAW);
    }

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

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glVertexAttribPointer(
        1,        // attribute
        2,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );
    check_gl_error();

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glVertexAttribPointer(
        2,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );
    check_gl_error();

    if (tangentBuffer != 0xffffffff)
    {
        // 3rd attribute buffer : tangents
        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
        glVertexAttribPointer(
            3,        // attribute
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );
        check_gl_error();

        // 3rd attribute buffer : binormals
        glEnableVertexAttribArray(4);
        glBindBuffer(GL_ARRAY_BUFFER, biTangentBuffer);
        glVertexAttribPointer(
            4,        // attribute
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );
        check_gl_error();
    }

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

Mesh::Mesh() : modelMatrix(glm::mat4(1.0))
{
    vertexBuffer = 0xffffffff;
    uvBuffer = 0xffffffff;
    normalBuffer = 0xffffffff;
    tangentBuffer = 0xffffffff;
    biTangentBuffer = 0xffffffff;
    elementBuffer = 0xffffffff;
}

Mesh::~Mesh()
{
    if (vertexBuffer != 0xcd)
        glDeleteBuffers(1, &vertexBuffer);
    if (uvBuffer != 0xcd)
        glDeleteBuffers(1, &uvBuffer);
    if (normalBuffer != 0xcd)
        glDeleteBuffers(1, &normalBuffer);
    if (elementBuffer != 0xcd)
        glDeleteBuffers(1, &elementBuffer);
}
