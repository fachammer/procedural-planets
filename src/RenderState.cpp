#include "RenderState.hpp"

void RenderState::setParameters(const ShaderEffect *effect, const glm::vec3 &lightPositionWorldSpace)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId);
    glUniform1i(effect->textureSamplerId, 0);
    check_gl_error();
    glUniform3f(effect->lightPositionId, lightPositionWorldSpace.x, lightPositionWorldSpace.y, lightPositionWorldSpace.z);
}
