#pragma once

#include <GL/glew.h>

unsigned int LoadShaders(const char * vertex_file_name,const char * fragment_file_name, const char* contentPath);
unsigned int LoadShaders(const char * vertex_file_name, const char * geometry_file_name, const char * fragment_file_name, const char* contentPath);


class AllShaderEffect
{
public:
	unsigned int programId;
	unsigned int MVPId;
	unsigned int VId;
	unsigned int MId;
	unsigned int textureSamplerId;
	unsigned int fluxId;
	unsigned int ndotl_ndotvSamplerId;
	unsigned int ndotl_ndothSamplerId;
	unsigned int ndotl_vdotlSamplerId;
	unsigned int cubeSamplerId;
	unsigned int lightPositionId;
	unsigned int specColorId;
	unsigned int shininessId;
	unsigned int metalnessId;
	unsigned int specularityId;
    unsigned int lightMatrixId;
    unsigned int constantColorId;
    unsigned int isShadowCasterId;
	unsigned int shadowMagicNumberId;
	unsigned int volumeSamplerId;
	//AllShaderEffect(unsigned int programId);
};


struct ShaderEffect
{
	ShaderEffect(unsigned int programId);
	unsigned int programId;
	unsigned int MVPId;
	unsigned int VId;
	unsigned int MId;
	unsigned int lightMatrixId;
};

struct SimpleShaderEffect : public ShaderEffect
{
	SimpleShaderEffect(unsigned int programId);
	unsigned int textureSamplerId;
	unsigned int lightPositionId;
};

struct VolumeRenderingShaderEffect : public ShaderEffect
{
	VolumeRenderingShaderEffect(unsigned int programId);
	unsigned int volumeSamplerId;
	unsigned int layerId;
};

// DONE created a shadowmapping shader effect
// this can pass the id of a depth texture to the fragment shader
// and  a shadowmagicnumber (which controls the shadow map bias)
struct ShadowMappingShaderEffect : public SimpleShaderEffect
{
	ShadowMappingShaderEffect(unsigned int programId);
	unsigned int depthTextureSamplerId;
	unsigned int ssaoResultSamplerId;
	unsigned int shadowMagicNumberId;
};


