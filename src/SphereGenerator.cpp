//
//  SphereGenerator.cpp
//  Tutorials
//
//  Created by Fabian Achammer on 05.12.15.
//
//

#include "SphereGenerator.hpp"
#include <common/mesh.hpp>
#include <GL/glew.h>
#include <math.h>

using namespace glm;

static const GLdouble theta = 0.5 * (1.0 + sqrt(5.0));
static const vec3 ICOSAHEDRON_VERTICES[12] = {
    vec3(-1, theta, 0),
    vec3(1, theta, 0),
    vec3(-1, -theta, 0),
    vec3(1, -theta, 0),

    vec3(0, -1, theta),
    vec3(0, 1, theta),
    vec3(0, -1, -theta),
    vec3(0, 1, -theta),

    vec3(theta, 0, -1),
    vec3(theta, 0, 1),
    vec3(-theta, 0, -1),
    vec3(-theta, 0, 1)};

static const unsigned int ICOSAHEDRON_INDICES[60] = {
    0, 11, 5,
    0, 5, 1,
    0, 1, 7,
    0, 7, 10,
    0, 10, 11,

    1, 5, 9,
    5, 11, 4,
    11, 10, 2,
    10, 7, 6,
    7, 1, 8,

    3, 9, 4,
    3, 4, 2,
    3, 2, 6,
    3, 6, 8,
    3, 8, 9,

    4, 9, 5,
    2, 4, 11,
    6, 2, 10,
    8, 6, 7,
    9, 8, 1};

static int addSphereVertex(Mesh *mesh, vec3 vertex, GLfloat radius)
{
    vec3 normalizedVertex = normalize(vertex * radius);
    mesh->indexed_vertices.push_back(normalizedVertex * radius);
    mesh->indexed_normals.push_back(normalizedVertex);
    mesh->indexed_uvs.push_back(glm::vec2(0.5f, 0.5f));
    return mesh->indexed_vertices.size() - 1;
}

Mesh *generateSphere(GLfloat radius, int subdivisions, bool reversedFaces)
{
    Mesh *sphere = new Mesh();

    for (int i = 0; i < 12; i++)
    {
        addSphereVertex(sphere, ICOSAHEDRON_VERTICES[i], radius);
    }

    for (int i = 0; i < 60; i++)
        sphere->indices.push_back(ICOSAHEDRON_INDICES[i]);

    for (int s = 0; s < subdivisions; s++)
    {
        std::vector<unsigned int> subdividedSphereIndices;

        for (int i = 0; (i + 2) < sphere->indices.size(); i += 3)
        {
            int aIndex = sphere->indices[i];
            int bIndex = sphere->indices[i + 1];
            int cIndex = sphere->indices[i + 2];

            vec3 a = sphere->indexed_vertices[aIndex];
            vec3 b = sphere->indexed_vertices[bIndex];
            vec3 c = sphere->indexed_vertices[cIndex];

            vec3 ab = a + b;
            vec3 bc = b + c;
            vec3 ca = c + a;

            int abIndex = addSphereVertex(sphere, ab, radius);
            int bcIndex = addSphereVertex(sphere, bc, radius);
            int caIndex = addSphereVertex(sphere, ca, radius);

            subdividedSphereIndices.push_back(aIndex);
            subdividedSphereIndices.push_back(abIndex);
            subdividedSphereIndices.push_back(caIndex);

            subdividedSphereIndices.push_back(bIndex);
            subdividedSphereIndices.push_back(bcIndex);
            subdividedSphereIndices.push_back(abIndex);

            subdividedSphereIndices.push_back(cIndex);
            subdividedSphereIndices.push_back(caIndex);
            subdividedSphereIndices.push_back(bcIndex);

            subdividedSphereIndices.push_back(abIndex);
            subdividedSphereIndices.push_back(bcIndex);
            subdividedSphereIndices.push_back(caIndex);
        }

        sphere->indices = subdividedSphereIndices;
    }

    if (reversedFaces)
    {
        std::vector<unsigned int> reversedIndices;

        for (int i = 0; i < sphere->indices.size(); i += 3)
        {
            for (int j = 2; j >= 0; j--)
            {
                reversedIndices.push_back(sphere->indices[i + j]);
            }
        }

        sphere->indices = reversedIndices;
    }

    return sphere;
}
