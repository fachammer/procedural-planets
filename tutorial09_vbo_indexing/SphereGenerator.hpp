//
//  SphereGenerator.h
//  Tutorials
//
//  Created by Fabian Achammer on 19.11.15.
//
//

#ifndef SphereGenerator_h
#define SphereGenerator_h

class Mesh;

#include <GL/glew.h>

Mesh* generateSphere(GLfloat radius, int subdivisions, bool reversed);

#endif /* SphereGenerator_h */
