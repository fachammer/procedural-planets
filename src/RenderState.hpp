#pragma once

#include "Shader.hpp"
#include "GLError.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>

struct RenderState
{
    unsigned int meshId;
    std::vector<int> shaderEffectIds;
    unsigned int texId;

    void setParameters(const ShaderEffect *effect, const glm::vec3 &lightPositionWorldSpace);
};
