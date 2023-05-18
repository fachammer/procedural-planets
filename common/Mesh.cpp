#include "Mesh.hpp"

#include <common/GLError.h>

#include <common/vboindexer.hpp>
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
        // return memcmp((void*)this, (void*)&that, sizeof(glm::vec3) + sizeof(glm::vec2))>0;
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

void Mesh::createQuad(glm::vec2 ll, glm::vec2 ur)
{
    indexed_vertices.push_back(glm::vec3(ll.x, ll.y, 0.0f));
    indexed_vertices.push_back(glm::vec3(ur.x, ll.y, 0.0f));
    indexed_vertices.push_back(glm::vec3(ll.x, ur.y, 0.0f));
    indexed_vertices.push_back(glm::vec3(ur.x, ur.y, 0.0f));

    indexed_uvs.push_back(glm::vec2(0.0f, 0.0f));
    indexed_uvs.push_back(glm::vec2(1.0f, 0.0f));
    indexed_uvs.push_back(glm::vec2(0.0f, 1.0f));
    indexed_uvs.push_back(glm::vec2(1.0f, 1.0f));

    indexed_normals.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
    indexed_normals.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
    indexed_normals.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
    indexed_normals.push_back(glm::vec3(0.0f, 0.0f, -1.0f));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(3);
}

void Mesh::createCube(glm::vec3 dimensions, bool frontFacing)
{
    indexed_vertices.push_back(glm::vec3(0, 0, 0));
    indexed_vertices.push_back(glm::vec3(1, 0, 0));
    indexed_vertices.push_back(glm::vec3(1, 0, 1));
    indexed_vertices.push_back(glm::vec3(0, 0, 1));

    indexed_vertices.push_back(glm::vec3(0, 1, 0));
    indexed_vertices.push_back(glm::vec3(1, 1, 0));
    indexed_vertices.push_back(glm::vec3(1, 1, 1));
    indexed_vertices.push_back(glm::vec3(0, 1, 1));

    indexed_vertices.push_back(glm::vec3(1, 0, 0));
    indexed_vertices.push_back(glm::vec3(1, 1, 0));
    indexed_vertices.push_back(glm::vec3(1, 1, 1));
    indexed_vertices.push_back(glm::vec3(1, 0, 1));

    indexed_vertices.push_back(glm::vec3(0, 0, 0));
    indexed_vertices.push_back(glm::vec3(0, 1, 0));
    indexed_vertices.push_back(glm::vec3(0, 1, 1));
    indexed_vertices.push_back(glm::vec3(0, 0, 1));

    indexed_vertices.push_back(glm::vec3(0, 0, 1));
    indexed_vertices.push_back(glm::vec3(1, 0, 1));
    indexed_vertices.push_back(glm::vec3(1, 1, 1));
    indexed_vertices.push_back(glm::vec3(0, 1, 1));

    indexed_vertices.push_back(glm::vec3(0, 0, 0));
    indexed_vertices.push_back(glm::vec3(1, 0, 0));
    indexed_vertices.push_back(glm::vec3(1, 1, 0));
    indexed_vertices.push_back(glm::vec3(0, 1, 0));

    for (int i = 0; i < 3; i++)
    {
        indexed_uvs.push_back(glm::vec2(0, 0));
        indexed_uvs.push_back(glm::vec2(1, 0));
        indexed_uvs.push_back(glm::vec2(1, 1));
        indexed_uvs.push_back(glm::vec2(0, 1));

        indexed_uvs.push_back(glm::vec2(0, 0));
        indexed_uvs.push_back(glm::vec2(1, 0));
        indexed_uvs.push_back(glm::vec2(1, 1));
        indexed_uvs.push_back(glm::vec2(0, 1));
    }

    for (int i = 0; i < 4; i++)
    {
        indexed_normals.push_back(glm::vec3(0, 1, 0));
    }

    for (int i = 0; i < 4; i++)
    {
        indexed_normals.push_back(glm::vec3(0, -1, 0));
    }

    for (int i = 0; i < 4; i++)
    {
        indexed_normals.push_back(glm::vec3(1, 0, 0));
    }

    for (int i = 0; i < 4; i++)
    {
        indexed_normals.push_back(glm::vec3(-1, 0, 0));
    }

    for (int i = 0; i < 4; i++)
    {
        indexed_normals.push_back(glm::vec3(0, 0, 1));
    }

    for (int i = 0; i < 4; i++)
    {
        indexed_normals.push_back(glm::vec3(0, 0, -1));
    }

    int base = 0;
    for (int i = 0; i < 3; i++)
    {
        base = i * 8;
        indices.push_back(base + 1);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 3);
        indices.push_back(base + 2);

        base = i * 8 + 4;
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
}

void Mesh::indexMesh()
{
    indexVBOWithTangents(vertices, uvs, normals, tangents, biTangents, indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_biTangents);
}

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

void getAverageNormal(
    PackedVertexWithTangents2 &packed,
    std::multimap<PackedVertexWithTangents2, unsigned int> &vertexToOutIndex,
    PackedVertexWithTangents2 &result)
{
    result.position = packed.position;
    result.uv = packed.uv;
    std::multimap<PackedVertexWithTangents2, unsigned int>::iterator it;
    int count = vertexToOutIndex.count(packed);

    for (it = vertexToOutIndex.equal_range(packed).first; it != vertexToOutIndex.equal_range(packed).second; ++it)
    {
        result.normal += it->first.normal;
        result.tangent += it->first.tangent;
        result.biTangent += it->first.biTangent;
    }

    result.normal = glm::normalize(result.normal);
    result.biTangent = glm::normalize(result.biTangent);
    result.tangent = glm::normalize(result.tangent);
}

void Mesh::calculateTangents()
{
    using namespace glm;
    unsigned int triangleCount = indices.size() / 3;
    for (unsigned int i = 0; i < triangleCount; i++)
    {

        vec3 tangent;
        vec3 bitangent;
        vec3 normal;

        unsigned int index0 = indices[i * 3];
        unsigned int index1 = indices[i * 3 + 1];
        unsigned int index2 = indices[i * 3 + 2];

        vec3 v0 = indexed_vertices[index0];
        vec3 v1 = indexed_vertices[index1];
        vec3 v2 = indexed_vertices[index2];

        vec2 uv0 = indexed_uvs[index0];
        vec2 uv1 = indexed_uvs[index1];
        vec2 uv2 = indexed_uvs[index2];

        vec3 P = v1 - v0;
        vec3 Q = v2 - v0;

        float s1 = uv1.x - uv0.x;
        float t1 = uv1.y - uv0.y;
        float s2 = uv2.x - uv0.x;
        float t2 = uv2.y - uv0.y;

        float tmp = 0.0f;
        if (fabsf(s1 * t2 - s2 * t1) <= 0.0001f)
        {
            tmp = 1.0f;
        }
        else
        {
            tmp = 1.0f / (s1 * t2 - s2 * t1);
        }

        tangent.x = (t2 * P.x - t1 * Q.x);
        tangent.y = (t2 * P.y - t1 * Q.y);
        tangent.z = (t2 * P.z - t1 * Q.z);

        tangent = tangent * tmp;

        bitangent.x = (s1 * Q.x - s2 * P.x);
        bitangent.y = (s1 * Q.y - s2 * P.y);
        bitangent.z = (s1 * Q.z - s2 * P.z);

        bitangent = bitangent * tmp;

        // normals.push_back(glm::normalize(indexed[i*3]));
        tangents.push_back(glm::normalize(tangent));
        biTangents.push_back(glm::normalize(bitangent));

        // normals.push_back(glm::normalize(normals[i*3+1]));
        tangents.push_back(glm::normalize(tangent));
        biTangents.push_back(glm::normalize(bitangent));

        // normals.push_back(glm::normalize(normals[i*3+2]));
        tangents.push_back(glm::normalize(tangent));
        biTangents.push_back(glm::normalize(bitangent));
    }

    indexed_tangents.resize(indexed_vertices.size());
    indexed_biTangents.resize(indexed_vertices.size());

    for (int i = 0; i < indices.size(); i++)
    {
        indexed_tangents[indices[i]] += tangents[i];
        indexed_biTangents[indices[i]] += biTangents[i];
    }

    for (int i = 0; i < indexed_tangents.size(); i++)
    {
        glm::normalize(indexed_tangents[i]);
        glm::normalize(indexed_biTangents[i]);
    }

    /*std::multimap<PackedVertexWithTangents2,unsigned int> verticesMap;
    for (unsigned int i = 0; i < vertices.size(); i++)
    {
        PackedVertexWithTangents2 packed = {vertices[i], uvs[i], normals[i], tangents[i], biTangents[i]};
        verticesMap.insert(std::pair<PackedVertexWithTangents2,unsigned int>(packed, i));
    }

    for (unsigned int i = 0; i < vertices.size(); i++)
    {
        PackedVertexWithTangents2 packed = {vertices[i], uvs[i],normals[i], tangents[i], biTangents[i]};
        PackedVertexWithTangents2 result = {vec3(0.0, 0.0, 0.0), vec2(0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0)};

        getAverageNormal(packed,verticesMap,result);

        normals[i] = result.normal;
        tangents[i] = result.tangent;
        biTangents[i] = result.biTangent;
    }*/
}

Mesh::Mesh()
{
    vertexBuffer = 0xffffffff;
    uvBuffer = 0xffffffff;
    ;
    normalBuffer = 0xffffffff;
    ;
    tangentBuffer = 0xffffffff;
    ;
    biTangentBuffer = 0xffffffff;
    ;
    elementBuffer = 0xffffffff;
    ;
    modelMatrix = glm::mat4(1.0);
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
