#pragma once

#include "Shader.hpp"
#include "GLError.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>

// Using Inheritance out of pure laziness
// Inheritance is not the right pattern for this task
// should be done with composition

struct RenderState
{
    unsigned int meshId;
    std::vector<int> shaderEffectIds;
    unsigned int texId;

    static glm::vec3 lightPositionWorldSpace;

    virtual void setParameters(ShaderEffect *effect)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texId);
        glUniform1i(effect->textureSamplerId, 0);
        check_gl_error();
        glUniform3f(effect->lightPositionId, lightPositionWorldSpace.x, lightPositionWorldSpace.y, lightPositionWorldSpace.z);
    }
};
