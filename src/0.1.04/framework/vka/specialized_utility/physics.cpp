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

			ComputeCmd cmd = ComputeCmd(particleCount, cVkaShaderPath + "physics/compute_cell_key.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC);
			cmd.pushLocal();
			if (desc.dimensions == PD_3D)
			{
				cmd.pipelineDef.shaderDef.args.push_back({ "PARTICLE_3D","" });
			}
			//std::string structDef = desc.getStructDef();
			//std::string structDef = "structParticle{vec2pos;uintplaceholder8;uintplaceholder12;};";
			//cmd.pipelineDef.shaderDef.args.push_back({ "PARTICLE_TYPE", structDef });

			struct PushConstants {
		        uint32_t taskSize;
				float radius;
				uint32_t structureSize;
				uint32_t structureOffset;
			} pc;
	        pc.taskSize        = particleCount;
			pc.radius = desc.radius;
			pc.structureSize = desc.structureSize;
			pc.structureOffset = desc.posAttributeOffset;
			pc.radius = desc.radius;
			cmd.pushConstant(&pc, sizeof(PushConstants));
			cmd.pushDescriptor(particle, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(cellKeys, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			return cmd;
		};

		ComputeCmd getCmdComputeStartId(Buffer cellKeys, Buffer startIndices, uint32_t count)
		{
	        ComputeCmd cmd = ComputeCmd(count, cVkaShaderPath + "physics/compute_start_id.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC);
			cmd.pushLocal();
			struct PushConstants {
		        uint32_t taskSize;
	        } pc;
	        pc.taskSize = count;
	        cmd.pushConstant(&pc, sizeof(PushConstants));
			cmd.pushDescriptor(cellKeys, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(startIndices, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        return cmd;
        }

		ComputeCmd getCmdComputeDensity(Buffer posBuf, const NeighborhoodIterator &it, const DensityComputeInfo &densityCI, Buffer densityBuf)
		{
	        densityBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	        uint32_t particleCount = posBuf->getSize() / densityCI.particleDesc.structureSize;
	        densityBuf->changeSize(particleCount * sizeof(float));
	        densityBuf->recreate();
	        ComputeCmd cmd = ComputeCmd(particleCount, cVkaShaderPath + "physics/compute_density.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC);
	        it.bind(cmd, densityCI.particleDesc);
	        cmd.pushLocal();
	        cmd.pushDescriptor(posBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(densityBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        struct PC
	        {
		        uint32_t taskSize;
		        float    radius;
		        uint32_t structureSize;
		        uint32_t structureOffset;
		        float    densityCoef;
	        } pc;
	        pc.taskSize        = particleCount;
	        pc.radius          = densityCI.particleDesc.radius;
	        pc.structureSize   = densityCI.particleDesc.structureSize;
	        pc.structureOffset = densityCI.particleDesc.posAttributeOffset;
	        pc.densityCoef     = densityCI.densityCoef;
	        cmd.pushConstant(&pc, sizeof(PC));
	        cmd.pipelineDef.shaderDef.args.push_back({"SELECT_KERNEL_TYPE", std::to_string(densityCI.kernelType)});
	        cmd.pipelineDef.shaderDef.args.push_back({"VECN_DIM", densityCI.particleDesc.dimensions == PD_2D ? "2" : "3"});
	        return cmd;
		}

		ComputeCmd getCmdAddPressureForce(Buffer posBuf, Buffer densityBuf, const NeighborhoodIterator& it, const PressureComputeInfo& pressureCI, Buffer pressureForceBuf)
		{
	        uint32_t particleCount = posBuf->getSize() / pressureCI.particleDesc.structureSize;
	        ComputeCmd cmd = ComputeCmd(particleCount, cVkaShaderPath + "physics/compute_pressure_force.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC);
	        it.bind(cmd, pressureCI.particleDesc);
	        cmd.pushLocal();
	        cmd.pushDescriptor(posBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(densityBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(pressureForceBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        struct PC
	        {
				uint32_t taskSize;
		        float    radius;
		        uint32_t structureSize;
		        uint32_t structureOffset;
		        float    forceCoef;
		        float    targetDensity;
	        } pc;
			pc.taskSize        = particleCount;
	        pc.radius          = pressureCI.particleDesc.radius;
	        pc.structureSize   = pressureCI.particleDesc.structureSize;
	        pc.structureOffset = pressureCI.particleDesc.posAttributeOffset;
	        pc.forceCoef       = pressureCI.forceCoef;
	        pc.targetDensity   = pressureCI.targetDensity;
	        cmd.pushConstant(&pc, sizeof(PC));
	        cmd.pipelineDef.shaderDef.args.push_back({"SELECT_KERNEL_TYPE", std::to_string(pressureCI.kernelType)});
	        cmd.pipelineDef.shaderDef.args.push_back({"VECN_DIM", pressureCI.particleDesc.dimensions == PD_2D ? "2" : "3"});
	        return cmd;
        }


		ComputeCmd getCmdAddViscosityForce(Buffer posBuf, Buffer densityBuf, Buffer velocityBuf, const NeighborhoodIterator &it, const ViscosityComputeInfo &ci, Buffer forceBuf)
        {
	        uint32_t   particleCount = posBuf->getSize() / ci.particleDesc.structureSize;
	        ComputeCmd cmd           = ComputeCmd(particleCount, cVkaShaderPath + "physics/compute_viscosity_force.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC);
	        it.bind(cmd, ci.particleDesc);
	        cmd.pushLocal();
	        cmd.pushDescriptor(posBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(densityBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(velocityBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(forceBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	        struct PC
	        {
		        uint32_t taskSize;
		        float    radius;
		        uint32_t structureSize;
		        uint32_t structureOffset;
		        float    forceCoef;
	        } pc;
	        pc.taskSize        = particleCount;
	        pc.radius          = ci.particleDesc.radius;
	        pc.structureSize   = ci.particleDesc.structureSize;
	        pc.structureOffset = ci.particleDesc.posAttributeOffset;
	        pc.forceCoef       = ci.forceCoef;
	        cmd.pushConstant(&pc, sizeof(PC));
	        cmd.pipelineDef.shaderDef.args.push_back({"SELECT_KERNEL_TYPE", std::to_string(ci.kernelType)});
	        cmd.pipelineDef.shaderDef.args.push_back({"VECN_DIM", ci.particleDesc.dimensions == PD_2D ? "2" : "3"});
	        return cmd;
        }

	}
}