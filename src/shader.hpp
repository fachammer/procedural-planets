#pragma once

#include <GL/glew.h>

struct Shader
{
private:
    GLuint shaderId = 0;

public:
    Shader(GLenum shaderType, std::string shaderCode)
    {
        shaderId = glCreateShader(shaderType);
        const char *shaderCodeCStr = shaderCode.c_str();
        glShaderSource(shaderId, 1, &shaderCodeCStr, NULL);
        glCompileShader(shaderId);
    }

    Shader(const Shader &shader) = delete;
    Shader operator=(const Shader &shader) = delete;

    ~Shader()
    {
        glDeleteShader(shaderId);
    }

    GLuint id() const
    {
        return shaderId;
    }
};

struct ShaderProgram
{
private:
    GLuint programId;
    std::vector<Shader *> shaders;

public:
    ShaderProgram(std::vector<Shader *> shaders)
    {
        programId = glCreateProgram();
        for (Shader *shader : shaders)
        {
            glAttachShader(programId, shader->id());
        }
        glLinkProgram(programId);
    }

    ShaderProgram(const ShaderProgram &shader) = delete;
    ShaderProgram operator=(const ShaderProgram &shader) = delete;

    ~ShaderProgram()
    {
        for (Shader *shader : shaders)
        {
            delete shader;
        }
        glDeleteProgram(programId);
    }

    GLuint id() const
    {
        return programId;
    }
};

ShaderProgram *LoadShaders(std::string vertexShaderPath, std::string fragmentShaderPath);
unsigned int LoadShaders(const char *vertex_file_name, const char *geometry_file_name, const char *fragment_file_name);
