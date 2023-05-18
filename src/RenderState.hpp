#pragma once

#include "shader.hpp"
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
    static glm::vec3 lightPositionWorldSpace2;

    virtual void setParameters(ShaderEffect *e)
    {
        SimpleShaderEffect *effect = static_cast<SimpleShaderEffect *>(e);
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texId);
        // Set our "myTextureSampler" sampler to user Texture Unit 0
        glUniform1i(effect->textureSamplerId, 0);
        check_gl_error();
        glUniform3f(effect->lightPositionId, lightPositionWorldSpace.x, lightPositionWorldSpace.y, lightPositionWorldSpace.z);
        glUniform3f(effect->lightPosition2Id, lightPositionWorldSpace2.x, lightPositionWorldSpace2.y, lightPositionWorldSpace2.z);
    }
};
