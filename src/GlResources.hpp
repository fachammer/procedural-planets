#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <SOIL.h>
#include <fstream>
#include <iostream>

using namespace std;

void _check_gl_error(const char *file, int line)
{
    GLenum err(glGetError());

    while (err != GL_NO_ERROR)
    {
        string error;

        switch (err)
        {
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }

        cerr << "GL_" << error.c_str() << " - " << file << ":" << line << endl;
        err = glGetError();
    }
}

#define check_gl_error() _check_gl_error(__FILE__, __LINE__)

class GlTexture
{
private:
    GLuint textureId;

public:
    GlTexture(std::string path)
    {
        textureId = SOIL_load_OGL_texture(
            path.c_str(),
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS);
    }
    GlTexture(const GlTexture &) = delete;
    GlTexture &operator=(const GlTexture &) = delete;

    GlTexture(GlTexture &&other) : textureId(other.textureId)
    {
        other.textureId = 0;
    }

    GlTexture &operator=(GlTexture &&other)
    {
        if (this != &other)
        {
            textureId = other.textureId;
            other.textureId = 0;
        }
        return *this;
    }

    ~GlTexture()
    {
        glDeleteTextures(1, &textureId);
    }

    GLuint id() const
    {
        return textureId;
    }
};

class GlVertexBuffer
{
private:
    unsigned int bufferId;

public:
    GlVertexBuffer(const std::vector<glm::vec3> &vertices)
    {
        glGenBuffers(1, &bufferId);
        glBindBuffer(GL_ARRAY_BUFFER, bufferId);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    }
    ~GlVertexBuffer()
    {
        glDeleteBuffers(1, &bufferId);
    }

    GlVertexBuffer(GlVertexBuffer &buffer) = delete;
    GlVertexBuffer operator=(GlVertexBuffer &buffer) = delete;

    GlVertexBuffer(GlVertexBuffer &&buffer) : bufferId(buffer.bufferId)
    {
        buffer.bufferId = 0;
    }

    GlVertexBuffer &operator=(GlVertexBuffer &&buffer)
    {
        if (this != &buffer)
        {
            bufferId = buffer.bufferId;
            buffer.bufferId = 0;
        }
        return *this;
    }

    unsigned int id() const
    {
        return bufferId;
    }
};

class GlElementBuffer
{
private:
    unsigned int bufferId;

public:
    GlElementBuffer(const std::vector<unsigned int> &indices)
    {
        glGenBuffers(1, &bufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    }
    ~GlElementBuffer()
    {
        glDeleteBuffers(1, &bufferId);
    }

    GlElementBuffer(GlElementBuffer &buffer) = delete;
    GlElementBuffer operator=(GlElementBuffer &buffer) = delete;

    GlElementBuffer(GlElementBuffer &&buffer) : bufferId(buffer.bufferId)
    {
        buffer.bufferId = 0;
    }

    GlElementBuffer &operator=(GlElementBuffer &&buffer)
    {
        if (this != &buffer)
        {
            bufferId = buffer.bufferId;
            buffer.bufferId = 0;
        }
        return *this;
    }

    unsigned int id() const
    {
        return bufferId;
    }
};

class GlMesh
{
private:
    GlVertexBuffer vertexBuffer;
    GlElementBuffer elementBuffer;
    unsigned int numberOfElements;

public:
    GlMesh(const std::vector<glm::vec3> &vertices, const std::vector<unsigned int> &indices)
        : vertexBuffer(vertices),
          elementBuffer(indices),
          numberOfElements(indices.size())
    {
    }

    GlMesh(const GlMesh &) = delete;
    GlMesh &operator=(const GlMesh &) = delete;

    GlMesh(GlMesh &&mesh) = default;
    GlMesh &operator=(GlMesh &&mesh) = default;

    const GlVertexBuffer &getVertexBuffer() const
    {
        return vertexBuffer;
    }

    const GlElementBuffer &getElementBuffer() const
    {
        return elementBuffer;
    }

    unsigned int getNumberOfElements() const
    {
        return numberOfElements;
    }
};

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

GlShader loadShader(GLenum shaderType, std::string path)
{
    std::ifstream shaderStream(path);
    std::string shaderCode((std::istreambuf_iterator<char>(shaderStream)),
                           (std::istreambuf_iterator<char>()));

    GLint Result = GL_FALSE;
    int InfoLogLength;

    GlShader shader = GlShader(shaderType, shaderCode);

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

GlShaderProgram
createVertexFragmentShaderProgram(GlShader vertexShader, GlShader fragmentShader)
{
    std::vector<GlShader> shaders;
    shaders.push_back(std::move(vertexShader));
    shaders.push_back(std::move(fragmentShader));
    GlShaderProgram shaderProgram = GlShaderProgram(shaders);

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

class GlVertexArrayObject
{
    GLuint vertexArrayId;

public:
    GlVertexArrayObject()
    {
        glGenVertexArrays(1, &vertexArrayId);
    }

    ~GlVertexArrayObject()
    {
        glDeleteVertexArrays(1, &vertexArrayId);
    }

    GLuint id() const
    {
        return vertexArrayId;
    }
};

struct Glfw
{
    Glfw()
    {
        int initResult = glfwInit();
        if (!initResult)
        {
            throw initResult;
        }
    }

    Glfw(const Glfw &) = delete;
    Glfw operator=(const Glfw &) = delete;

    ~Glfw()
    {
        glfwTerminate();
    }
};

struct Glew
{
    Glew()
    {
        glewExperimental = true;
        int initResult = glewInit();
        if (initResult != GLEW_OK)
        {
            throw initResult;
        }
    }
};

struct GlfwWindow
{
private:
    GLFWwindow *window;

public:
    GlfwWindow(unsigned int initialWidth, unsigned int initialHeight, const std::string &title)
    {
        glfwWindowHint(GLFW_SAMPLES, 8);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(initialWidth, initialHeight, title.c_str(), NULL, NULL);
        if (window == NULL)
        {
            throw -1;
        }
        glfwMakeContextCurrent(window);

        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        glfwSetCursorPos(window, initialWidth / 2, initialHeight / 2);
    }

    ~GlfwWindow()
    {
        glfwDestroyWindow(window);
    }

    GLFWwindow *glfwWindow() const
    {
        return window;
    }
};
