#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "Shader.hpp"
#include "GLError.h"

Shader loadShader(GLenum shaderType, std::string path)
{
    std::ifstream shaderStream(path);
    std::string shaderCode((std::istreambuf_iterator<char>(shaderStream)),
                           (std::istreambuf_iterator<char>()));

    GLint Result = GL_FALSE;
    int InfoLogLength;

    Shader shader = Shader(shaderType, shaderCode);

    glGetShaderiv(shader.id(), GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shader.id(), GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> shaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(shader.id(), InfoLogLength, NULL, &shaderErrorMessage[0]);
        printf("%s\n", &shaderErrorMessage[0]);
    }

    return shader;
}

ShaderProgram
createVertexFragmentShaderProgram(Shader vertexShader, Shader fragmentShader)
{
    std::vector<Shader> shaders;
    shaders.push_back(std::move(vertexShader));
    shaders.push_back(std::move(fragmentShader));
    ShaderProgram shaderProgram = ShaderProgram(shaders);

    GLint Result = GL_FALSE;
    int InfoLogLength;

    glGetProgramiv(shaderProgram.id(), GL_LINK_STATUS, &Result);
    glGetProgramiv(shaderProgram.id(), GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(shaderProgram.id(), InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    return shaderProgram;
}
