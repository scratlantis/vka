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

#ifndef MAX_PARTICLES_PER_CELL
#define MAX_PARTICLES_PER_CELL 1024
#endif


#if VECN_DIM == 2
#define NEIGHBORHOOD_CELL_COUNT 9
#else
#define NEIGHBORHOOD_CELL_COUNT 27
#endif


#define ITERATE_START(POS,ID)							\
	ivecN originCell = getCell(POS, pni_uniform.radius);\
	for(uint i = 0; i<NEIGHBORHOOD_CELL_COUNT; i++)		\
	{													\
		ivecN neighborCell = applyOffset(originCell,i);	\
		uint hash = hashCell(neighborCell);				\
		uint key = keyFromHash(hash, pni_uniform.range);\
		uint startID = pni_start_id[key];				\
		if(startID == 0xFFFFFFFF)						\
		{												\
			continue;									\
		}												\
		for(uint j = 0; j<MAX_PARTICLES_PER_CELL; j++)	\
		{												\
			if(pni_cell_keys[startID + j] != key)		\
			{											\
				break;									\
			}											\
			uint otherId = pni_permutation[startID + j];\
			if(ID == otherId)							\
			{											\
				continue;								\
			}											\
			// CODE(otherId)
#define ITERATE_END \
		}		    \
	}

#endif // PARTICLE_NEIGHBORHOOD_ITERATOR_H