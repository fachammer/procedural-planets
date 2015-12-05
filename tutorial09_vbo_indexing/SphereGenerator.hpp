//
//  SphereGenerator.h
//  Tutorials
//
//  Created by Fabian Achammer on 19.11.15.
//
//

#ifndef SphereGenerator_h
#define SphereGenerator_h

#include <common/mesh.hpp>
#include <GL/glew.h>
#include <math.h>

Mesh* generateSphere(GLfloat radius, int subdivisions) {
    Mesh* sphere = new Mesh();
    GLdouble theta = 26.56505117707799 * M_PI / 180.0;
    GLdouble sinTheta = std::sin(theta);
    GLdouble cosTheta = std::cos(theta);
    
    sphere->indexed_vertices.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
    sphere->indexed_normals.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
    GLfloat phi = M_PI / 5.0;
    for(int i = 1; i < 6; i++){
        glm::vec3 point(cosTheta * std::cos(phi), cosTheta * std::sin(phi), sinTheta);
        sphere->indexed_vertices.push_back(point);
        sphere->indexed_normals.push_back(point);
        phi += 2 * M_PI / 5.0;
    }
    
    phi = 0.0;
    for (int i = 6; i < 11; ++i) {
        glm::vec3 point(cosTheta * std::cos(phi), cosTheta * std::sin(phi), sinTheta);
        sphere->indexed_vertices.push_back(point);
        sphere->indexed_normals.push_back(point);
        
        phi += 2.0 * M_PI / 5.0;
    }
    
    sphere->indexed_vertices.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    sphere->indexed_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    
    unsigned int indices[60] = {
        0, 2, 1, 0, 3, 2, 0, 4, 3,
        0, 5, 4, 0, 1, 5, 1, 2, 7,
        2, 3, 8, 3, 4, 9, 4, 5, 10,
        5, 1, 6, 1, 7, 6, 2, 8, 7,
        3, 9, 8, 4, 10, 9, 5, 6, 10,
        6, 7, 11, 7, 8, 11, 8, 9, 11,
        9, 10, 11, 10, 6, 11
    };
    
    for(int i = 0; i < 60; i++){
        sphere->indices.push_back(indices[i]);
    }
    
    for(int i = 0; i < 12; i++)
        sphere->indexed_uvs.push_back(glm::vec2(0.5f, 0.5f));
    
    return sphere;
}


#endif /* SphereGenerator_h */
