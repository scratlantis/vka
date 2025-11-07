#include "interface_structs.h"
#include "../shader_interface.h"
#include "ui.h"
#include "resources.h"

using namespace physics;


void SimulationResources::init(IResourcePool *pPool, uint32_t preallocCount)
{
	it.init(pPool, preallocCount);
	densityBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	forceBuffer   = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

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
	return it.isInitialized() && densityBuffer && forceBuffer;
}

Rect2D<float> getViewAdjustedArea()
{
	VkRect2D_OP   targetImgSize = getScissorRect(viewDimensions);
	float         maxDim        = float(max(targetImgSize.extent.width, targetImgSize.extent.height));
	Rect2D<float> area{
	    0.f,
	    0.f,
	    float(targetImgSize.extent.width) / maxDim,
	    float(targetImgSize.extent.height) / maxDim};
	return area;
}

GVar gvar_particle_generation_seed{"Particle Seed", 42U, GVAR_UINT_RANGE, GUI_CAT_PARTICLE_GEN, {0U, 100U}};
GVar gvar_particle_generation_count{"Particle Count", 1000U, GVAR_UINT_RANGE, GUI_CAT_PARTICLE_GEN, {1U, 100000U}};
GVar gvar_particle_size{"Particle Size", 0.01f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.1f, 10.f}};

ComputeCmd getCmdGenParticles(ParticleResources *pRes)
{
	ComputeCmd cmd(pRes->count(), shaderPath + "generat_particles.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC, {});
	cmd.pushLocal();
	cmd.pushDescriptor(pRes->getParticleBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->getPredictedPosBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->getVelocityBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	if (pRes->desc.dimensions == PD_3D)
	{
		struct PushStruct
		{
			vec3  minRange;
			uint  pad0;

			vec3  maxRange;
			uint  pad1;

			uint  seed;
			uint  taskSize;
			float radius;
		} pc;
		Rect3D<float> area = {0.f,0.f,0.f,1.f,1.f,1.f};
		pc.minRange        = vec3(area.x, area.y, area.z);
		pc.maxRange        = vec3(area.x + area.width, area.y + area.height, area.z + area.depth);
		pc.seed            = gvar_particle_generation_seed.val.v_uint;
		pc.taskSize        = pRes->count();
		pc.radius          = pRes->desc.radius;
		cmd.pushConstant(&pc, sizeof(PushStruct));
		cmd.pipelineDef.shaderDef.args.push_back({ "VECN_DIM","3" });
	}
	else
	{
		struct PushStruct
		{
			vec2  minRange;
			vec2  maxRange;
			uint  seed;
			uint  taskSize;
			float radius;
		} pc;
		Rect2D<float> area = getViewAdjustedArea();
		pc.minRange        = vec2(area.x, area.y);
		pc.maxRange        = vec2(area.x + area.width, area.y + area.height);
		pc.seed            = gvar_particle_generation_seed.val.v_uint;
		pc.taskSize        = pRes->count();
		pc.radius          = pRes->desc.radius;
		cmd.pushConstant(&pc, sizeof(PushStruct));
	}
	return cmd;
}

void cmdReorderParticles(CmdBuffer cmdBuf, ParticleResources *pRes)
{
	ComputeCmd cmd = ComputeCmd(pRes->count(), shaderPath + "reorder_particles.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC);
	cmd.pushLocal();
	cmd.pushDescriptor(pRes->getParticleBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->getPredictedPosBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->simRes.it.getPermutationBuffer(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->getParticlePingPongBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->getPredictedPosPingPongBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	struct PC
	{
		uint32_t taskSize;
	} pc;
	cmd.pipelineDef.shaderDef.args.push_back({"VECN_DIM", pRes->desc.dimensions == PD_2D ? "2" : "3"});
	cmd.exec(cmdBuf);
	pRes->swapPingPongBuffers();
}


GVar gvar_mouse_influence_radius{"Mouse Influence Radius", 0.1f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.01f, 1.f}};
GVar gvar_mouse_influence_strength{"Mouse Influence Strength", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 10.f}};

GVar gvar_particle_update_damping{"Particle Damping", 0.98f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 1.0f}};
GVar gvar_particle_update_damping_border{"Particle Damping Border", 0.98f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 1.0f}};
GVar gvar_particle_update_gravity{"Particle Gravity", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 10.0f}};
GVar gvar_particle_update_inter_gravity{"Inter-Particle Gravity", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 10.0f}};




ComputeCmd getCmdUpdateParticles(ParticleResources *pRes, float timeStep, uint32_t frameInvocationNr)
{
	ComputeCmd cmd(pRes->count(), shaderPath + "update_particles.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC, {});
	cmd.pushLocal();
	cmd.pushDescriptor(pRes->getParticleBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->simRes.forceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->getPredictedPosBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->getVelocityBuf(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pRes->getParamsBuf(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	if (pRes->desc.dimensions == PD_3D)
	{
		struct PushStruct
		{
			vec3 minRange;
			uint pad0;

			vec3 maxRange;
			uint pad1;

			vec3 cursorPos;
			uint pad2;

			uvec2 cursorButtons;
			float cursorRadius;
			float cursorStrength;

			float radius;
			float dt;
			float damping;
			float dampingBorder;

			float gravity;
			uint  taskSize;
			uint  frameInvocationNr;
			float particleGravityCoef;
		} pc;

		Rect3D<float> area = {0.f,0.f,0.f,1.f,1.f,1.f};
		pc.particleGravityCoef = gvar_particle_update_inter_gravity.val.v_float;
		pc.frameInvocationNr = frameInvocationNr;
		pc.minRange        = vec3(area.x, area.y, area.x);
		pc.maxRange        = vec3(area.x + area.width, area.y + area.height, area.z + area.depth);
		pc.cursorPos       = vec3(0.5f); // todo
		pc.cursorButtons.x  = (mouseInView(viewDimensions) && gState.io.mouse.leftPressed) ? 1U : 0U;
		pc.cursorButtons.y  = (mouseInView(viewDimensions) && gState.io.mouse.rightPressed) ? 1U : 0U;
		pc.cursorRadius     = gvar_mouse_influence_radius.val.v_float;
		pc.cursorStrength   = mouseInView(viewDimensions) ? gvar_mouse_influence_strength.val.v_float : 0.0f;
		pc.radius          = pRes->desc.radius;
		pc.dt              = timeStep;
		pc.damping         = gvar_particle_update_damping.val.v_float;
		pc.dampingBorder   = gvar_particle_update_damping_border.val.v_float;
		pc.gravity         = gvar_particle_update_gravity.val.v_float;
		pc.taskSize        = pRes->count();
		cmd.pushConstant(&pc, sizeof(PushStruct));
		cmd.pipelineDef.shaderDef.args.push_back({"VECN_DIM", "3"});
	}
	else
	{
		struct PushStruct
		{
			vec2 minRange;
			vec2 maxRange;

			vec2 cursorPos;
			uint pad3;
			uint pad4;

			uvec2 cursorButtons;
			float cursorRadius;
			float cursorStrength;

			float radius;
			float dt;
			float damping;
			float dampingBorder;

			float gravity;
			uint  taskSize;
			uint  frameInvocationNr;
			float particleGravityCoef;
		} pc;
		Rect2D<float> area = getViewAdjustedArea();
		pc.particleGravityCoef = gvar_particle_update_inter_gravity.val.v_float;
		pc.frameInvocationNr = frameInvocationNr;
		pc.minRange        = vec2(area.x, area.y);
		pc.maxRange        = vec2(area.x + area.width, area.y + area.height);
		pc.cursorPos       = mouseInView(viewDimensions) ? mouseViewCoord(viewDimensions) : vec2(0.0f);
		pc.cursorButtons.x  = (mouseInView(viewDimensions) && gState.io.mouse.leftPressed) ? 1U : 0U;
		pc.cursorButtons.y  = (mouseInView(viewDimensions) && gState.io.mouse.rightPressed) ? 1U : 0U;
		pc.cursorRadius     = gvar_mouse_influence_radius.val.v_float;
		pc.cursorStrength   = mouseInView(viewDimensions) ? gvar_mouse_influence_strength.val.v_float : 0.0f;
		pc.radius          = pRes->desc.radius;
		pc.dt              = timeStep;
		pc.damping         = gvar_particle_update_damping.val.v_float;
		pc.dampingBorder   = gvar_particle_update_damping_border.val.v_float;
		pc.gravity         = gvar_particle_update_gravity.val.v_float;
		pc.taskSize        = pRes->count();
		cmd.pushConstant(&pc, sizeof(PushStruct));
		cmd.pipelineDef.shaderDef.args.push_back({"VECN_DIM", "2"});
	}
	return cmd;
}



GVar gvar_particle_density_coef{"Particle Density Coefficient", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.01f, 1.f}};
GVar gvar_particle_pressure_force_coef{"Particle pressure force coefficient", 0.01f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 0.5f}};
GVar gvar_particle_viscosity_force_coef{"Particle viscosity force coefficient", 0.01f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 10.0f}};
GVar gvar_particle_target_density{"Particle Target Density", 10.f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 1.f}};




GVar gvar_simulation_time_scale{"Simulation Time Scale", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_GENERAL, {0.0f, 10.f}};
void cmdComputeInteractionData(CmdBuffer cmdBuf, ParticleResources *pRes)
{
	{
		cmdInitBuffer(cmdBuf, pRes->simRes.forceBuffer, 0U, pRes->count() * pRes->descAttr.structureSize);
		pRes->simRes.it.cmdUpdate(cmdBuf, pRes->getPredictedPosBuf(), pRes->descAttr);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	{
		DensityComputeInfo densityCI{};
		densityCI.particleDesc = pRes->descAttr;
		densityCI.kernelType   = SK_QUADRATIC;
		densityCI.densityCoef  = gvar_particle_density_coef.val.v_float;
		getCmdComputeDensity(pRes->getPredictedPosBuf(), pRes->simRes.it, densityCI, pRes->simRes.densityBuffer).exec(cmdBuf);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	{
		PressureComputeInfo pressureCI{};
		pressureCI.particleDesc  = pRes->descAttr;
		pressureCI.kernelType    = SK_QUADRATIC;
		pressureCI.forceCoef     = gvar_particle_pressure_force_coef.val.v_float;
		pressureCI.targetDensity = gvar_particle_target_density.val.v_float;
		getCmdAddPressureForce(pRes->getPredictedPosBuf(), pRes->simRes.densityBuffer, pRes->simRes.it, pressureCI, pRes->simRes.forceBuffer).exec(cmdBuf);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT);

	{
		ViscosityComputeInfo viscosityCI{};
		viscosityCI.particleDesc = pRes->descAttr;
		viscosityCI.kernelType   = SK_SMOOTH;
		viscosityCI.forceCoef    = gvar_particle_viscosity_force_coef.val.v_float;
		getCmdAddViscosityForce(pRes->getPredictedPosBuf(), pRes->simRes.densityBuffer, pRes->getVelocityBuf(), pRes->simRes.it, viscosityCI, pRes->simRes.forceBuffer).exec(cmdBuf);
	}

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_READ_BIT);

	//cmdReorderParticles(cmdBuf, pRes);
}

GVar gvar_simulation_step_count{"Simulation Steps Per Frame", 1U, GVAR_UINT_RANGE, GUI_CAT_PARTICLE_UPDATE, {1U, 10U}};



void cmdSimulateParticles(CmdBuffer cmdBuf, ParticleResources *pRes, GLSLParticleUpdateParams params, uint32_t frameInvocationNr)
{

	cmdWriteCopy(cmdBuf, pRes->getParamsBuf(), &params, sizeof(GLSLParticleUpdateParams));
	cmdComputeInteractionData(cmdBuf, pRes);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	float timeStep = min(static_cast<float>(gState.frameTime), 20.0f) * (1.f / static_cast<float>(gvar_simulation_step_count.val.v_uint));
	timeStep *= gvar_simulation_time_scale.val.v_float;
	getCmdUpdateParticles(pRes, timeStep, frameInvocationNr).exec(cmdBuf);
}
