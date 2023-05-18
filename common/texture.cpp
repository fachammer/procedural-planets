#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "GLError.h"

#include <glfw3.h>

#include <SOIL.h>
#include <string>

GLuint loadSoil(const char *imagename, const char *contentPath)
{
    std::string imagePath = std::string(contentPath) + std::string(imagename);
    GLuint tex_ID = SOIL_load_OGL_texture(
        imagePath.c_str(),
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS
        //| SOIL_FLAG_MULTIPLY_ALPHA
        //| SOIL_FLAG_COMPRESS_TO_DXT
        //| SOIL_FLAG_DDS_LOAD_DIRECT
        //| SOIL_FLAG_NTSC_SAFE_RGB
        //| SOIL_FLAG_CoCg_Y
        //| SOIL_FLAG_TEXTURE_RECTANGLE
    );
    check_gl_error();
    glBindTexture(GL_TEXTURE_2D, tex_ID);
    check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    check_gl_error();
    return tex_ID;
}
