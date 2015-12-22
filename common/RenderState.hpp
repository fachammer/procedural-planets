#pragma once


#include "shader.hpp"
#include <common/GLError.h>
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
//struct RenderState
//{
//	unsigned int meshId;
//	unsigned int shaderEffectId;
//	unsigned int textureId;
//	unsigned int depthMapId;
//	unsigned int worldspacePosTexId;
//	unsigned int worldspaceNormalTexId;
//	unsigned int fluxTexId;
//	unsigned int cubeTexId;
//	unsigned int volumeTexId;
//	float shininess;
//	float metalness;
//	float specularity;
//	glm::vec3 specularMaterialColor;
//    int isShadowCaster;
//
//	RenderState() :
//		shininess(8.0f),
//			metalness(1.0f),
//			specularity(0.3f),
//			specularMaterialColor(glm::vec3(0.5, 0.5, 1.0))
//
//	{
//	}
//
//	void set(unsigned int mId, unsigned int sId, unsigned int tex1, 
//		unsigned int tex2, unsigned int wsTexId, unsigned int wsNormId, unsigned int fluxId)
//	{
//		meshId = mId;
//		shaderEffectId = sId;
//		textureId = tex1;
//		volumeTexId = tex2;
//		worldspacePosTexId = wsTexId;
//		worldspaceNormalTexId = wsNormId;
//		fluxTexId = fluxId;
//	}
//};



// Using Inheritance out of pure laziness
// Inheritance is not the right pattern for this task
// should be done with composition

struct RenderState
{
	unsigned int meshId;
    std::vector<int> shaderEffectIds;
	

	virtual void setParameters(ShaderEffect* effect) 
	{
		printf("shouldn't be here...\n");
	}
};

struct SimpleRenderState : public RenderState
{
	public: 
	unsigned int texId;
	static glm::vec3 lightPositionWorldSpace;

	virtual void setParameters(ShaderEffect* e)
	{
		SimpleShaderEffect* effect = static_cast<SimpleShaderEffect*>(e);
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texId);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(effect->textureSamplerId, 0);
		check_gl_error();
		glUniform3f(effect->lightPositionId, lightPositionWorldSpace.x, lightPositionWorldSpace.y, lightPositionWorldSpace.z );
	}
};

struct VolumeRenderingRenderState : public RenderState
{
	unsigned int volumeTexId;
	static unsigned int layerNr;

	virtual void setParameters(ShaderEffect* e)
	{
		VolumeRenderingShaderEffect* effect = static_cast<VolumeRenderingShaderEffect*>(e);
		glActiveTexture(GL_TEXTURE0);
		check_gl_error();
		glBindTexture(GL_TEXTURE_3D, volumeTexId);
		check_gl_error();
		glUniform1i(effect->volumeSamplerId, 0);
		check_gl_error();
		//glUniform1i(effect->layerId, layerNr);
		//check_gl_error();
	}
};

// DONE added the Shadow mapping render state
// it holds the application side values of the 
// ShadowMappingShaderEffectr
// the id of the depth texture and
// the bias magic number
struct ShadowMappingRenderState : public SimpleRenderState
{
	unsigned int depthTexId;
	unsigned int ssaoResultTexId;

	static float shadowMagicNumber;

	virtual void setParameters(ShaderEffect* e)
	{
		SimpleRenderState::setParameters(e);
		ShadowMappingShaderEffect* effect = static_cast<ShadowMappingShaderEffect*>(e);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTexId);
		glUniform1i(effect->depthTextureSamplerId, 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ssaoResultTexId);
		glUniform1i(effect->ssaoResultSamplerId, 2);
		check_gl_error();
		glUniform1f(effect->shadowMagicNumberId, shadowMagicNumber);
	}
};




