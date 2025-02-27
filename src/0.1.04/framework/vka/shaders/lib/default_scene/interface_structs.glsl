// Must be cpp compatible
#ifndef DEFAULT_SCENE_INTERFACE_STRUCTS_H
#define DEFAULT_SCENE_INTERFACE_STRUCTS_H

struct GLSLCamera
{
	mat4 projection;
	mat4 invProjection;
	uint width;
	uint height;
	uint padding[2];
};

struct GLSLCameraInstance
{
	mat4 view;
	mat4 invView;
	uint frameIdx;
	uint padding[3];
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

#define TYPE_DEFAUL 0
#define TYPE_LINE_SEGMENT 1
struct GLSLInstance
{
	mat4 mat;
	mat4 invMat;

	vec3 color;
	uint cullMask;

	uint id;
	uint padding[3];
};
#endif // INTERFACE_H
