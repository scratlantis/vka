#include "interface_structs.h"
#include "../shader_interface.h"
#include "ui.h"

using namespace physics;

void cmdGenParticles(CmdBuffer cmdBuf, Buffer particleBuffer, Buffer predictedPosBuffer, Buffer velocityBuffer, GenParticleArgs args)
{
	ComputeCmd cmd(args.particleCount, shaderPath + "generat_particles.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC, {});
	cmd.pushLocal();
	cmd.pushDescriptor(particleBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(predictedPosBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(velocityBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	PCGenerateParticles pc{};
	pc.x_range = vec2(args.area.x, args.area.x + args.area.width);
	pc.y_range = vec2(args.area.y, args.area.y + args.area.height);
	pc.seed = args.seed;
	pc.taskSize = args.particleCount;
	pc.radius = args.radius;
	cmd.pushConstant(&pc, sizeof(PCGenerateParticles));
	cmd.exec(cmdBuf);
}


GVar gvar_particle_generation_seed{"Particle Seed", 42U, GVAR_UINT_RANGE, GUI_CAT_PARTICLE_GEN, {0U, 100U}};
GVar gvar_particle_generation_count{"Particle Count", 1000U, GVAR_UINT_RANGE, GUI_CAT_PARTICLE_GEN, {1U, 100000U}};
GVar gvar_particle_size{"Particle Size", 0.01f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.1f, 1.f}};


void cmdGenParticles(CmdBuffer cmdBuf, Buffer particleBuffer, Buffer predictedPosBuffer, Buffer velocityBuffer)
{
	VkRect2D_OP targetImgSize = getScissorRect(viewDimensions);
	float maxDim = float(max(targetImgSize.extent.width, targetImgSize.extent.height));
	Rect2D<float> area{
		0.f,
		0.f,
		float(targetImgSize.extent.width) / maxDim,
		float(targetImgSize.extent.height) / maxDim };
	GenParticleArgs args{};
	args.particleCount = gvar_particle_generation_count.val.v_uint;
	args.area          = area;
	args.seed          = gvar_particle_generation_seed.val.v_uint;
	args.radius        = gvar_particle_size.val.v_float * cParticle_size_scale;
	cmdGenParticles(cmdBuf, particleBuffer, predictedPosBuffer, velocityBuffer, args);
}
GVar gvar_mouse_influence_radius{"Mouse Influence Radius", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.01f, 1.f}};
GVar gvar_mouse_influence_strength{"Mouse Influence Strength", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.1f, 10.f}};

ComputeCmd getCmdUpdateParticles(Buffer particleBuffer, Buffer forceBuffer, UpdateParticleArgs args, Buffer predictedPos, Buffer velocityBuffer)
{
	uint32_t particleCount = particleBuffer->getSize() / sizeof(GLSLParticle);
	ComputeCmd cmd(particleCount, shaderPath + "update_particles.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC, {});
	cmd.pushLocal();
	cmd.pushDescriptor(particleBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(forceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(predictedPos, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(velocityBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	PCUpdateParticles pc{};
	pc.x_range = vec2(args.bounds.x, args.bounds.x + args.bounds.width);
	pc.y_range = vec2(args.bounds.y, args.bounds.y + args.bounds.height);
	pc.damping = args.damping;
	pc.gravity = args.gravity;
	pc.dt = args.dt;
	pc.pointSize = args.particleSize;
	pc.taskSize = particleCount;

	pc.mouse_buttons.x = gState.io.mouse.leftPressed ? 1U : 0U;
	pc.mouse_buttons.y = gState.io.mouse.rightPressed ? 1U : 0U;
	pc.mouse_radius    = gvar_mouse_influence_radius.val.v_float;
	pc.mouse_influence = 0.0f;
	pc.mouse_pos       = vec2(0.0);
	if (mouseInView(viewDimensions) && gState.io.mouse.leftPressed || gState.io.mouse.rightPressed)
	{
		pc.mouse_pos = mouseViewCoord(viewDimensions);
		pc.mouse_influence = gvar_mouse_influence_strength.val.v_float;
	}
	//if (mouseView.x >)
	
	//pc.mouse_pos = gState.io.mouse.;

	cmd.pushConstant(&pc, sizeof(PCUpdateParticles));
	return cmd;
}

GVar gvar_particle_update_damping{"Particle Damping", 0.98f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 1.0f}};
GVar gvar_particle_update_damping_border{"Particle Damping Border", 0.98f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 1.0f}};

GVar gvar_particle_update_gravity{"Particle Gravity", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 10.0f}};

GVar gvar_particle_density_coef{"Particle Density Coefficient", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.1f, 10.f}};
GVar gvar_particle_pressure_force_coef{"Particle pressure force coefficient", 0.01f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 0.1f}};
GVar gvar_particle_viscosity_force_coef{"Particle viscosity force coefficient", 0.01f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 10.0f}};
GVar gvar_particle_target_density{"Particle Target Density", 10.f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 1.f}};


void SimulationResources::init(IResourcePool *pPool, uint32_t preallocCount)
{
	it.init(pPool, preallocCount);
	densityBuffer       = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	forceBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	if (preallocCount > 0)
	{
		densityBuffer->changeSize(preallocCount * sizeof(float));
		densityBuffer->recreate();
		densityBuffer->addFlags(BUFFER_FLAG_DONT_REDUCE);

		forceBuffer->changeSize(preallocCount * sizeof(vec2));
		forceBuffer->recreate();
		forceBuffer->addFlags(BUFFER_FLAG_DONT_REDUCE);
	}
}
bool SimulationResources::isInitialized() const
{
	return it.isInitialized()
		&& densityBuffer && forceBuffer;
}

GVar gvar_simulation_time_scale{"Simulation Time Scale", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_GENERAL, {0.1f, 10.f}};
void cmdApplyTimeStep(CmdBuffer cmdBuf, Buffer particleBuffer, Buffer forceBuffer, float timeStep, Buffer predictedPos, Buffer velocityBuffer)
{
	VkRect2D_OP   targetImgSize = getScissorRect(viewDimensions);
	float         maxDim        = float(max(targetImgSize.extent.width, targetImgSize.extent.height));
	Rect2D<float> area{
	    0.f,
	    0.f,
	    float(targetImgSize.extent.width) / maxDim,
	    float(targetImgSize.extent.height) / maxDim};

	UpdateParticleArgs updateArgs{};
	updateArgs.bounds                 = area;
	updateArgs.particleSize           = gvar_particle_size.val.v_float * cParticle_size_scale;
	updateArgs.dt                     = timeStep * gvar_simulation_time_scale.val.v_float;
	updateArgs.damping                = gvar_particle_update_damping.val.v_float;
	updateArgs.dampingBorderCollision = gvar_particle_update_damping_border.val.v_float;
	updateArgs.gravity                = gvar_particle_update_gravity.val.v_float;

	getCmdUpdateParticles(particleBuffer, forceBuffer, updateArgs, predictedPos, velocityBuffer).exec(cmdBuf);
}

void cmdComputeInteractionData(CmdBuffer cmdBuf, Buffer posBuffer, Buffer velocityBuffer, const ParticleDescription &desc, SimulationResources &res)
{
	{
		res.it.cmdUpdate(cmdBuf, posBuffer, desc);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	{
		DensityComputeInfo densityCI{};
		densityCI.particleDesc = desc;
		densityCI.kernelType   = SK_QUADRATIC;
		densityCI.densityCoef  = gvar_particle_density_coef.val.v_float;
		getCmdComputeDensity(posBuffer, res.it, densityCI, res.densityBuffer).exec(cmdBuf);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	uint32_t particleCount = posBuffer->getSize() / desc.structureSize;
	res.forceBuffer->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	res.forceBuffer->changeSize(particleCount * (desc.dimensions == PD_2D ? sizeof(glm::vec2) : sizeof(glm::vec3)));
	res.forceBuffer->recreate();
	cmdFillBuffer(cmdBuf, res.forceBuffer, 0U); // zero is zero

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	{
		PressureComputeInfo pressureCI{};
		pressureCI.particleDesc  = desc;
		pressureCI.kernelType    = SK_QUADRATIC;
		pressureCI.forceCoef     = gvar_particle_pressure_force_coef.val.v_float;
		pressureCI.targetDensity = gvar_particle_target_density.val.v_float;
		getCmdAddPressureForce(posBuffer, res.densityBuffer, res.it, pressureCI, res.forceBuffer).exec(cmdBuf);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT);


	{
		ViscosityComputeInfo viscosityCI{};
		viscosityCI.particleDesc = desc;
		viscosityCI.kernelType   = SK_SMOOTH;
		viscosityCI.forceCoef    = gvar_particle_viscosity_force_coef.val.v_float;
		getCmdAddViscosityForce(posBuffer, res.densityBuffer, velocityBuffer, res.it, viscosityCI, res.forceBuffer).exec(cmdBuf);

	}
}

void cmdSimulateParticles(CmdBuffer cmdBuf, Buffer particleBuffer, Buffer predictedPosBuffer, Buffer velocityBuffer, SimulationResources &res, float timeStep)
{
	cmdComputeInteractionData(cmdBuf, predictedPosBuffer, velocityBuffer, particle_type<glm::vec2>::get_description(gvar_particle_size.val.v_float * cParticle_size_scale), res);
	//cmdComputeInteractionData(cmdBuf, particleBuffer, particle_type<GLSLParticle>::get_description(gvar_particle_size.val.v_float), res);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	cmdApplyTimeStep(cmdBuf, particleBuffer, res.forceBuffer, timeStep, predictedPosBuffer, velocityBuffer);
}
