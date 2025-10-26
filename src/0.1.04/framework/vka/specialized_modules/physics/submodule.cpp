#include "submodule.h"
#include <vka/specialized_utility/physics.h>
#include <vka/specialized_utility/sorting.h>
#include <vka/globals.h>


namespace vka
{
	namespace physics
	{
    void NeighborhoodIterator::init(IResourcePool *pPool, uint32_t preallocCount)
		{
			cellKeys = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			startIndices = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			permutation = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			pingPongCellKeys = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			pingPongPermutation = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			histogram = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	        uniformBuf = createBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |VK_BUFFER_USAGE_TRANSFER_DST_BIT);

			if (preallocCount > 0)
			{
				cellKeys->changeSize(preallocCount * sizeof(uint32_t));
				cellKeys->recreate();
		        cellKeys->addFlags(BUFFER_FLAG_DONT_REDUCE);
			
				startIndices->changeSize(preallocCount * sizeof(uint32_t));
				startIndices->recreate();
		        startIndices->addFlags(BUFFER_FLAG_DONT_REDUCE);
			
				permutation->changeSize(preallocCount * sizeof(uint32_t));
				permutation->recreate();
		        permutation->addFlags(BUFFER_FLAG_DONT_REDUCE);
			
				pingPongCellKeys->changeSize(preallocCount * sizeof(uint32_t));
				pingPongCellKeys->recreate();
		        pingPongCellKeys->addFlags(BUFFER_FLAG_DONT_REDUCE);
			
				pingPongPermutation->changeSize(preallocCount * sizeof(uint32_t));
				pingPongPermutation->recreate();
		        pingPongPermutation->addFlags(BUFFER_FLAG_DONT_REDUCE);
			}
		}
		NeighborhoodIterator::NeighborhoodIterator(IResourcePool* pPool)
		{
			init(pPool);
		}

		void NeighborhoodIterator::cmdUpdate(CmdBuffer cmdBuf, Buffer particleBuf, const ParticleDescription &desc)
		{
			uint32_t particleCount = particleBuf->getSize() / desc.structureSize;
	        struct Uniforms
	        {
		        float radius;
		        uint32_t range;
	        } ubo;
			ubo.radius = desc.radius;
	        ubo.range  = particleCount;
	        cmdWriteCopy(cmdBuf, uniformBuf, &ubo, sizeof(ubo));
	        getCmdComputeCellKeys(particleBuf, cellKeys, desc).exec(cmdBuf);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	        cmdRadixSortPermutation(cmdBuf, cellKeys, permutation, particleCount,
	                                pingPongCellKeys, pingPongPermutation, histogram);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			startIndices->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	        cmdInitBuffer(cmdBuf, startIndices, 0xFFFFFFFF, sizeof(uint32_t) * particleCount);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	        getCmdComputeStartId(cellKeys, startIndices, particleCount).exec(cmdBuf);
		}

		void NeighborhoodIterator::bind(ComputeCmd &cmd, const ParticleDescription& desc) const
		{
			cmd.pushSubmodule(cVkaShaderModulePath + "physics/particle_neighborhood_iterator.glsl");
			cmd.pushDescriptor(permutation, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(cellKeys, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(startIndices, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(uniformBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			//cmd.pushSpecializationConst(desc.radius);
			//cmd.pushSpecializationConst(static_cast<uint32_t>(permutation->getSize() / sizeof(uint32_t)));

		}

	}
}