#ifndef PARTICLE_NEIGHBORHOOD_ITERATOR_H
#define PARTICLE_NEIGHBORHOOD_ITERATOR_H

#define PARTICLE_NEIGHBORHOOD_ITERATOR_H_BINDING_COUNT 3

#include "../../programs/physics/grid_hash.glsl"

layout(binding = PARTICLE_NEIGHBORHOOD_ITERATOR_BINDING_OFFSET + 0) readonly buffer PNI_PERMUTATION { uint pni_permutation[]; };
layout(binding = PARTICLE_NEIGHBORHOOD_ITERATOR_BINDING_OFFSET + 1) readonly buffer PNI_CELL_KEYS { uint pni_cell_keys[]; };
layout(binding = PARTICLE_NEIGHBORHOOD_ITERATOR_BINDING_OFFSET + 2) readonly buffer PNI_START_ID { uint pni_start_id[]; };
layout(constant_id = PARTICLE_NEIGHBORHOOD_ITERATOR_SPEC_CONST_OFFSET + 0) const float pni_radius = 1.0;
layout(constant_id = PARTICLE_NEIGHBORHOOD_ITERATOR_SPEC_CONST_OFFSET + 1) const uint pni_range = 1;



uint getStartID(vec3 pos, uint cellID)
{
	ivec3 centerCellPos = ivec3(floor(pos / (2.0 * pni_radius)));
	ivec3 cellOffset = ivec3(cellID%3, (cellID/3)%3, cellID/9) - ivec3(1);
	uint cellKey = hash(centerCellPos + cellOffset) % pni_range;
	return pni_start_id[cellKey];
}

uint getStartID(vec2 pos, uint cellID)
{
	ivec2 centerCellPos = ivec2(floor(pos / (2.0 * pni_radius)));
	ivec2 cellOffset = ivec2(cellID%3, (cellID/3)%3) - ivec2(1);
	uint cellKey = hash(centerCellPos + cellOffset) % pni_range;
	//cellKey = hash(centerCellPos) % pni_range;
	return pni_start_id[cellKey];
}

struct ParticleID
{
	uint id;
	uint cellID;
};

ParticleID getParticleID(uint startID, uint entryID)
{
	ParticleID pID;
	pID.cellID = pni_cell_keys[startID + entryID];
	pID.id = pni_permutation[startID + entryID];
	return pID;
}

#endif