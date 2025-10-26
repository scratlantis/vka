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

			std::string getStructDef() const;

		};
		ComputeCmd getCmdComputeCellKeys(Buffer particle, Buffer cellKeys, ParticleDescription desc);
		ComputeCmd getCmdComputeStartId(Buffer cellKeys, Buffer startIndices);

		enum ShaderKernelType
		{
			SK_QUADRATIC,
		};
		struct NeighborhoodIteratorResources;

		void cmdComputeParticleDensity(CmdBuffer cmdBuf, Buffer particleBuf, const ParticleDescription& desc,
			NeighborhoodIteratorResources& res, ShaderKernelType kernel, float densityCoef, float forceCoef, float targetDensity, glm::vec2 mouseCoord,
			Buffer densityBuf, Buffer pressureForce);
	}
}