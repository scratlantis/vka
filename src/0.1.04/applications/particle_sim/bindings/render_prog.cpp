#include "interface_structs.h"
#include "../shader_interface.h"
#include "ui.h"

void cmdRenderParticles(CmdBuffer cmdBuf, Image target, Buffer particleBuffer, RenderParticleArgs args)
{
	DrawCmd cmd = DrawCmd();
	cmd.renderArea = VkRect2D_OP(target->getExtent2D());

	addShader(cmd.pipelineDef, shaderPath + "render_particle.vert");
	addShader(cmd.pipelineDef, shaderPath + "render_particle.frag");
	cmd.pipelineDef.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

	DrawSurface ds{};
	ds.vertexBuffer = particleBuffer;
	ds.count = particleBuffer->getSize() / sizeof(GLSLParticle);
	ds.vertexLayout = vertex_type<GLSLParticle>{}.data_layout();
	cmd.setGeometry(ds);

	cmd.pushColorAttachment(target, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, BlendOperation::add(), BlendOperation::add());
	PCRenderParticles pc{};
	pc.viewOffset = vec2(0.f, 0.f);
	VkExtent2D extent = target->getExtent2D();
	float maxDim = float(max(extent.width, extent.height));
	pc.viewScale.x = maxDim / float(extent.width);
	pc.viewScale.y = maxDim / float(extent.height);
	pc.pointSize = args.particleSize * maxDim;
	pc.intensity = args.particleIntensity;
	pc.extent = vec2(static_cast<float>(extent.width),static_cast<float>(extent.height));
	cmd.pushConstant(&pc, sizeof(PCRenderParticles), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	cmd.exec(cmdBuf);
}

extern GVar gvar_particle_size;
GVar gvar_particle_rel_render_size{"Particle Render Size", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_RENDER, {1.0f, 100.f}};
GVar gvar_particle_render_brightness{"Particle Render Brightness", 0.5f, GVAR_FLOAT_RANGE, GUI_CAT_RENDER, {0.0f, 10.f}};

void cmdRenderParticles(CmdBuffer cmdBuf, Image target, Buffer particleBuffer)
{
	RenderParticleArgs args{};
	args.particleSize = gvar_particle_rel_render_size.val.v_float * gvar_particle_size.val.v_float;
	args.particleIntensity = gvar_particle_render_brightness.val.v_float;
	cmdRenderParticles(cmdBuf, target, particleBuffer, args);
}