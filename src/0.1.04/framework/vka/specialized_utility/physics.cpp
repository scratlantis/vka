#include "physics.h"
#include <vka/specialized_modules/physics/submodule.h>
namespace vka
{
	namespace physics
	{
		std::string ParticleDescription::getStructDef() const
		{
			VKA_ASSERT(dimensions == PD_2D && structureSize % 8 == 0
				|| dimensions == PD_3D && structureSize % 16 == 0);

			std::string structDef = "struct Particle {";
			for (uint32_t i = 0; i < structureSize; i += 4)
			{
				if (posAttributeOffset == i)
				{
					if (dimensions == PD_3D)
					{
						VKA_ASSERT(i % 16 == 0);
						structDef += "vec3 pos;";
						i += 8;
					}
					else
					{
						VKA_ASSERT(i % 8 == 0);
						structDef += "vec2 pos;";
						i += 4;
					}
				}
				else
				{
					structDef += "uint placeholder" + std::to_string(i) + ";";
				}
			}
			structDef += "};";
			return structDef;
		}

		ComputeCmd getCmdComputeCellKeys(Buffer particle, Buffer cellKeys, ParticleDescription desc)
		{
			uint32_t particleCount = particle->getSize() / desc.structureSize;
			cellKeys->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			cellKeys->changeSize(particleCount * sizeof(uint32_t));
			cellKeys->recreate();

			ComputeCmd cmd = ComputeCmd(particleCount, cVkaShaderPath + "physics/compute_cell_key.comp");
			cmd.pushLocal();
			if (desc.dimensions == PD_3D)
			{
				cmd.pipelineDef.shaderDef.args.push_back({ "PARTICLE_3D","" });
			}
			//std::string structDef = desc.getStructDef();
			//std::string structDef = "structParticle{vec2pos;uintplaceholder8;uintplaceholder12;};";
			//cmd.pipelineDef.shaderDef.args.push_back({ "PARTICLE_TYPE", structDef });

			struct PushConstants {
				float radius;
				uint32_t structureSize;
				uint32_t structureOffset;
			} pc;
			pc.radius = desc.radius;
			pc.structureSize = desc.structureSize;
			pc.structureOffset = desc.posAttributeOffset;
			pc.radius = desc.radius;
			cmd.pushConstant(&pc, sizeof(PushConstants));
			cmd.pushDescriptor(particle, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(cellKeys, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			return cmd;
		};

		ComputeCmd getCmdComputeStartId(Buffer cellKeys, Buffer startIndices)
		{
			ComputeCmd cmd = ComputeCmd(cellKeys->getSize() / sizeof(uint32_t), cVkaShaderPath + "physics/compute_start_id.comp");
			cmd.pushLocal();
			cmd.pushDescriptor(cellKeys, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(startIndices, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			return cmd;
		}

		void cmdComputeParticleDensity(CmdBuffer cmdBuf, Buffer particleBuf, const ParticleDescription& desc,
			NeighborhoodIteratorResources& res, SmoothingKernel kernel, float densityCoef, glm::vec2 mouseCoord, Buffer densityBuf)
		{
			densityBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			uint32_t particleCount = particleBuf->getSize() / desc.structureSize;
			densityBuf->changeSize(particleCount * sizeof(float));
			densityBuf->recreate();

			cmdUpdateNeighborhoodIterator(cmdBuf, particleBuf, desc, res);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

			ComputeCmd cmd = ComputeCmd(particleCount, cVkaShaderPath + "physics/compute_density.comp");
			bindNeighborhoodIterator(cmd, desc, res);
			cmd.pushLocal();
			cmd.pushDescriptor(particleBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(densityBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			struct PCDensity {
				float radius;
				uint32_t structureSize;
				uint32_t structureOffset;
				float densityCoef;
				glm::vec2 mouseCoord;
			} pc;
			pc.radius = desc.radius;
			pc.structureSize = desc.structureSize;
			pc.structureOffset = desc.posAttributeOffset;
			pc.densityCoef = densityCoef;
			pc.mouseCoord = mouseCoord;
			cmd.pushConstant(&pc, sizeof(PCDensity));
			if (kernel == SK_SQUARE_CUBED)
			{
				cmd.pipelineDef.shaderDef.args.push_back({ "SMOOTHING_KERNEL_SQUARE_CUBED","" });
			}
			cmd.exec(cmdBuf);
		}
		
	}
}