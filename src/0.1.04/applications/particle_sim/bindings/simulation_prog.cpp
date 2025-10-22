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


GVar gvar_particle_generation_seed{"Particle Seed", 42U, GVAR_UINT_RANGE, GUI_CAT_PARTICLES, {0U, 100U}};
GVar gvar_particle_generation_count{"Particle Count", 10U, GVAR_UINT_RANGE, GUI_CAT_PARTICLES, {1U, 100U}};
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
