#ifndef PARTICLE_NEIGHBORHOOD_ITERATOR_H
#define PARTICLE_NEIGHBORHOOD_ITERATOR_H

#include "../../programs/physics/grid_hash.glsl"
#define PARTICLE_NEIGHBORHOOD_ITERATOR_H_BINDING_COUNT 4
layout(binding = PARTICLE_NEIGHBORHOOD_ITERATOR_BINDING_OFFSET + 0) readonly buffer PNI_PERMUTATION { uint pni_permutation[]; };
layout(binding = PARTICLE_NEIGHBORHOOD_ITERATOR_BINDING_OFFSET + 1) readonly buffer PNI_CELL_KEYS { uint pni_cell_keys[]; };
layout(binding = PARTICLE_NEIGHBORHOOD_ITERATOR_BINDING_OFFSET + 2) readonly buffer PNI_START_ID { uint pni_start_id[]; };

struct PNI_Uniform
{
	float radius;
	uint range;
};
layout(binding = PARTICLE_NEIGHBORHOOD_ITERATOR_BINDING_OFFSET + 3) uniform UNIFORM { PNI_Uniform pni_uniform; };



uint getStartID(vec3 pos, uint cellID)
{
	ivec3 centerCellPos = ivec3(floor(pos / (2.0 * pni_uniform.radius)));
	ivec3 cellOffset = ivec3(cellID%3, (cellID/3)%3, cellID/9) - ivec3(1);
	uint cellKey = hash(centerCellPos + cellOffset) % pni_uniform.range;
	return pni_start_id[cellKey];
}

uint getStartID(vec2 pos, uint cellID)
{
	ivec2 centerCellPos = ivec2(floor(pos / (2.0 * pni_uniform.radius)));
	ivec2 cellOffset = ivec2(cellID%3, (cellID/3)%3) - ivec2(1);
	uint cellKey = hash(centerCellPos + cellOffset) % pni_uniform.range;
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

#ifndef MAX_PARTICLES_PER_CELL
#define MAX_PARTICLES_PER_CELL 32
#endif


#if VECN_DIM == 2
#define NEIGHBORHOOD_CELL_COUNT 9
#else
#define NEIGHBORHOOD_CELL_COUNT 27
#endif

#define ITERATE_START									\
	[[ unroll ]]										\
	for(uint i = 0; i<NEIGHBORHOOD_CELL_COUNT; i++)		\
	{													\
		uint startID = getStartID(localPos, i);			\
		uint lastCellKey = 0xFFFFFFFF;					\
		[[ unroll ]]									\
		for(uint j = 0; j<MAX_PARTICLES_PER_CELL; j++)	\
		{												\
			ParticleID pID = getParticleID(startID, j);	\
			if(j!= 0 && pID.cellID != lastCellKey)		\
			{											\
				break;									\
			}											\
			lastCellKey = pID.cellID;					\
			if(pID.id != id)							\
			{
				// CODE(pID)
#define ITERATE_END \
			}		\
		}		    \
	}





#endif