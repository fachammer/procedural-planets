#pragma once

#include <GL/glew.h>

unsigned int LoadShaders(const char *vertex_file_name, const char *fragment_file_name);
unsigned int LoadShaders(const char *vertex_file_name, const char *geometry_file_name, const char *fragment_file_name);

struct ShaderEffect
{
    ShaderEffect(unsigned int programId);
    unsigned int programId;
    unsigned int MVPId;
    unsigned int VId;
    unsigned int MId;
    unsigned int textureSamplerId;
    unsigned int lightDirectionId;
};
