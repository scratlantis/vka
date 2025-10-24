#include "interface_structs.h"
#include "../shader_interface.h"
#include "ui.h"

void cmdGenParticles(CmdBuffer cmdBuf, Buffer particleBuffer, GenParticleArgs args)
{
	particleBuffer->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	particleBuffer->changeSize(args.particleCount * sizeof(GLSLParticle));
	particleBuffer->recreate();

	ComputeCmd cmd(args.particleCount, shaderPath + "generat_particles.comp", {});
	cmd.pushLocal();
	cmd.pushDescriptor(particleBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	PCGenerateParticles pc{};
	pc.x_range = vec2(args.area.x, args.area.x + args.area.width);
	pc.y_range = vec2(args.area.y, args.area.y + args.area.height);
	pc.seed = args.seed;
	cmd.pushConstant(&pc, sizeof(PCGenerateParticles));
	cmd.exec(cmdBuf);
}


GVar gvar_particle_generation_seed{"Particle Seed", 42U, GVAR_UINT_RANGE, GUI_CAT_PARTICLE_GEN, {0U, 100U}};
GVar gvar_particle_generation_count{"Particle Count", 100U, GVAR_UINT_RANGE, GUI_CAT_PARTICLE_GEN, {1U, 1000U}};
void cmdGenParticles(CmdBuffer cmdBuf, Buffer particleBuffer)
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
	cmdGenParticles(cmdBuf, particleBuffer, args);
}

void cmdUpdateParticles(CmdBuffer cmdBuf, Buffer particleBuffer, UpdateParticleArgs args)
{
	uint32_t particleCount = particleBuffer->getSize() / sizeof(GLSLParticle);
	ComputeCmd cmd(particleCount, shaderPath + "update_particles.comp", {});
	cmd.pushLocal();
	cmd.pushDescriptor(particleBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	PCUpdateParticles pc{};
	pc.x_range = vec2(args.bounds.x, args.bounds.x + args.bounds.width);
	pc.y_range = vec2(args.bounds.y, args.bounds.y + args.bounds.height);
	pc.damping = args.damping;
	pc.gravity = args.gravity;
	pc.dt = args.dt;
	pc.pointSize = args.particleSize;
	cmd.pushConstant(&pc, sizeof(PCUpdateParticles));
	cmd.exec(cmdBuf);
}

GVar gvar_particle_update_damping{"Particle Damping", 0.98f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 1.0f}};
GVar gvar_particle_update_gravity{"Particle Gravity", 0.0f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.0f, 1.0f}};
GVar gvar_particle_size{"Particle Size", 0.01f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.001f, 0.5f}};

void cmdUpdateParticles(CmdBuffer cmdBuf, Buffer particleBuffer)
{
	VkRect2D_OP targetImgSize = getScissorRect(viewDimensions);
	float maxDim = float(max(targetImgSize.extent.width, targetImgSize.extent.height));
	Rect2D<float> area{
		0.f,
		0.f,
		float(targetImgSize.extent.width) / maxDim,
		float(targetImgSize.extent.height) / maxDim };
	UpdateParticleArgs args{};
	args.bounds       = area;
	args.particleSize = gvar_particle_size.val.v_float;
	args.dt           = gState.frameTime;
	args.damping      = gvar_particle_update_damping.val.v_float;
	args.gravity      = gvar_particle_update_gravity.val.v_float;
	cmdUpdateParticles(cmdBuf, particleBuffer, args);
}

GVar gvar_particle_density_coef{"Particle Density Coefficient", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_PARTICLE_UPDATE, {0.1f, 10.f}};

void cmdUpdateParticleDensity(CmdBuffer cmdBuf, Buffer particleBuffer, physics::NeighborhoodIteratorResources res, Buffer densityBuffer)
{
	physics::ParticleDescription desc = particle_type<GLSLParticle>::get_description(gvar_particle_size.val.v_float);
	physics::cmdComputeParticleDensity(cmdBuf, particleBuffer, desc, res, physics::SK_SQUARE_CUBED,
		gvar_particle_density_coef.val.v_float, densityBuffer);
}