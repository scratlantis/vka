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
	uint  instanceIndex;

	vec3  normal;
	uint  padding1[1];

	vec3  v0;
	uint  padding2[1];

	vec3  v1;
	uint  padding3[1];

	vec3  v2;
	float intensity; // Area * intensity

	vec3  color;
	uint  padding4[1];

};

struct GLSLAccelerationStructureInstanceKHR
{
    float	transform[12];
    uint    instanceCustomIndex24_mask8;
    uint	instanceShaderBindingTableRecordOffset24_flags8;
    uint    asRef1;
    uint    asRef2;
};

#endif // VKA_STRUCTS_H