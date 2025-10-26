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
	        float              radius;
	        ParticleDimensions dimensions;
	        uint32_t           structureSize;
	        uint32_t           posAttributeOffset;

			std::string getStructDef() const;

		};
		ComputeCmd getCmdComputeCellKeys(Buffer particle, Buffer cellKeys, ParticleDescription desc);
        ComputeCmd getCmdComputeStartId(Buffer cellKeys, Buffer startIndices, uint32_t count);

		enum ShaderKernelType
		{
			SK_QUADRATIC,
		};
		class NeighborhoodIterator;
        struct DensityComputeInfo
        {
	        ParticleDescription particleDesc;
	        ShaderKernelType    kernelType;
	        float               densityCoef;
        };
        ComputeCmd getCmdComputeDensity(Buffer particleBuf, const NeighborhoodIterator &it, const DensityComputeInfo &densityCI, Buffer densityBuf);

		struct PressureComputeInfo
		{
			ParticleDescription particleDesc;
			ShaderKernelType    kernelType;
			float               forceCoef;
			float               targetDensity;
		};
        ComputeCmd getCmdComputePressureForce(Buffer particleBuf, Buffer densityBuf, const NeighborhoodIterator &it, const PressureComputeInfo &pressureCI, Buffer pressureForceBuf);
	}
}