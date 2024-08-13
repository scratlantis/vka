
#ifndef VKA_STRUCTS_H
#define VKA_STRUCTS_H

struct VKAModelDataOffset
{
	uint firstVertex;
	uint firstSurface;
};

struct VKAAreaLight
{
	// NEE select light source
	vec3  center;
	uint  padding1[1];

	vec3  normal;
	uint  padding2[1];

	vec3  v0;
	uint  padding3[1];

	vec3  v1;
	uint  padding4[1];

	vec3  v2;
	float importance; // Area * intensity

};

#endif // VKA_STRUCTS_H