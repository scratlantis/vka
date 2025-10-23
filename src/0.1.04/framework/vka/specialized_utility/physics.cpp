#include "physics.h"

namespace vka
{
	namespace physics
	{
		ComputeCmd getCmdComputeCellKeys(Buffer particle, Buffer cellKeys, ParticleDescription desc)
		{
			uint32_t particleCount = particle->getSize() / desc.structureSize;
			cellKeys->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			cellKeys->changeSize(particleCount * sizeof(uint32_t));
			cellKeys->recreate();

			ComputeCmd cmd = ComputeCmd(particleCount, cVkaShaderPath + "physics/compute_cell_keys.comp");
			if (desc.dimensions == PD_3D)
			{
				cmd.pipelineDef.shaderDef.args.push_back({ "PARTICLE_3D","" });
			}

			VKA_ASSERT(desc.dimensions == PD_2D && desc.structureSize % 8 == 0
				|| desc.dimensions == PD_3D && desc.structureSize % 16 == 0);

			std::string structDef = "struct Particle {";
			for (uint32_t i = 0; i < desc.structureSize; i += 4)
			{
				if( desc.posAttributeOffset == i )
				{
					if (desc.dimensions == PD_3D)
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
			cmd.pipelineDef.shaderDef.args.push_back({ "PARTICLE_TYPE", structDef });

			struct PushConstants {
				float radius;
			} pc;
			pc.radius = desc.radius;
			cmd.pushConstant(&pc, sizeof(PushConstants));
			cmd.pushDescriptor(particle, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(cellKeys, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			return cmd;
		};

		ComputeCmd getCmdComputeStartId(Buffer cellKeys, Buffer startIndices)
		{
			startIndices->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
			startIndices->changeSize(cellKeys->getSize());
			startIndices->recreate();

			ComputeCmd cmd = ComputeCmd(cellKeys->getSize() / sizeof(uint32_t), cVkaShaderPath + "physics/compute_start_id.comp");
			cmd.pushDescriptor(cellKeys, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(startIndices, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			return cmd;
		}
	}
}