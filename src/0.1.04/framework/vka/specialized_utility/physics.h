#pragma once
#include <vka/advanced_utility/complex_commands.h>
namespace vka
{
	namespace physics
	{
		enum ParticleDimensions
		{
			PD_2D,
			PD_3D
		};
		struct ParticleDescription
		{
			float radius;
			ParticleDimensions dimensions;
			uint32_t structureSize;
			uint32_t posAttributeOffset;

		};
		ComputeCmd getCmdComputeCellKeys(Buffer particle, Buffer cellKeys, ParticleDescription desc);
		ComputeCmd getCmdComputeStartId(Buffer cellKeys, Buffer startIndices);
	}
}