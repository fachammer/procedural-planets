#include "RenderState.hpp"

unsigned int VolumeRenderingRenderState::layerNr = 0;
float ShadowMappingRenderState::shadowMagicNumber = 0.0f;
glm::vec3 SimpleRenderState::lightPositionWorldSpace = glm::vec3(20.0, 20.0, 20.0);