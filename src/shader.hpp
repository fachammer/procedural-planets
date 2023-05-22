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

    Shader(Shader &&shader)
        : shaderId(shader.shaderId)

    {
        shader.shaderId = 0;
    }

    Shader &operator=(Shader &&other)
    {
        if (this != &other)
        {
            shaderId = other.shaderId;
            other.shaderId = 0;
        }
        return *this;
    }

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

public:
    ShaderProgram(const std::vector<Shader> &shaders)
    {
        programId = glCreateProgram();
        for (const Shader &shader : shaders)
        {
            glAttachShader(programId, shader.id());
        }
        glLinkProgram(programId);
    }

    ShaderProgram(const ShaderProgram &) = delete;
    ShaderProgram operator=(const ShaderProgram &) = delete;

    ShaderProgram(ShaderProgram &&program)
        : programId(program.programId)

    {
        program.programId = 0;
    }

    ShaderProgram &operator=(ShaderProgram &&other)
    {
        if (this != &other)
        {
            programId = other.programId;
            other.programId = 0;
        }
        return *this;
    }

    ~ShaderProgram()
    {
        glDeleteProgram(programId);
    }

    GLuint id() const
    {
        return programId;
    }
};

Shader loadShader(GLenum shaderType, std::string path);
ShaderProgram createVertexFragmentShaderProgram(Shader vertexShader, Shader fragmentShader);
