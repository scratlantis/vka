// Must be cpp compatible
#ifndef INTERFACE_H
#define INTERFACE_H

struct GLSLFrame
{
	mat4 projection;
	mat4 invProjection;
	uint width;
	uint height;
	uint idx;
	uint padding[1];
};

struct GLSLView
{
	mat4 mat;
	mat4 invMat;
	vec4 pos;
};

struct GLSLParams
{
	uint padding[4];
};


struct GLSLVertex
{
	vec3 pos;
	uint padding_0[1];

	vec3 normal;
	uint padding_1[1];

	vec2 uv;
	uint padding_2[2];
};

struct GLSLMaterial
{
	vec3 albedo;
	uint padding_0[1];

	vec3 specular;
	uint padding_1[1];

	vec3 emission;
	uint padding_2[1];

	float roughness;
	float f0; //  0.16 reflectance^2
	uint padding_3[2];
};

#endif // INTERFACE_H
