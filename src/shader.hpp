#pragma once

#include <GL/glew.h>

struct GlShader
{
private:
    GLuint shaderId = 0;

public:
    GlShader(GLenum shaderType, std::string shaderCode)
    {
        shaderId = glCreateShader(shaderType);
        const char *shaderCodeCStr = shaderCode.c_str();
        glShaderSource(shaderId, 1, &shaderCodeCStr, NULL);
        glCompileShader(shaderId);
    }

    GlShader(const GlShader &) = delete;
    GlShader operator=(const GlShader &) = delete;

    GlShader(GlShader &&shader)
        : shaderId(shader.shaderId)

    {
        shader.shaderId = 0;
    }

    GlShader &operator=(GlShader &&other)
    {
        if (this != &other)
        {
            shaderId = other.shaderId;
            other.shaderId = 0;
        }
        return *this;
    }

    ~GlShader()
    {
        glDeleteShader(shaderId);
    }

    GLuint id() const
    {
        return shaderId;
    }
};

struct GlShaderProgram
{
private:
    GLuint programId;

public:
    GlShaderProgram(const std::vector<GlShader> &shaders)
    {
        programId = glCreateProgram();
        for (const GlShader &shader : shaders)
        {
            glAttachShader(programId, shader.id());
        }
        glLinkProgram(programId);
    }

    GlShaderProgram(const GlShaderProgram &) = delete;
    GlShaderProgram operator=(const GlShaderProgram &) = delete;

    GlShaderProgram(GlShaderProgram &&program)
        : programId(program.programId)

    {
        program.programId = 0;
    }

    GlShaderProgram &operator=(GlShaderProgram &&other)
    {
        if (this != &other)
        {
            programId = other.programId;
            other.programId = 0;
        }
        return *this;
    }

    ~GlShaderProgram()
    {
        glDeleteProgram(programId);
    }

    GLuint id() const
    {
        return programId;
    }
};

GlShader loadShader(GLenum shaderType, std::string path);
GlShaderProgram createVertexFragmentShaderProgram(GlShader vertexShader, GlShader fragmentShader);
