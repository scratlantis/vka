#include "shader_interface.h"
#include "ui.h"
#include "config.h"

GVar gvar_medium_density_scale{ "Noise scale", 0.2f, GVAR_FLOAT_RANGE, GUI_CAT_GENERAL, {0.01f, 10.f} };
GVar gvar_emission_scale_al{ "Area light emission scale", 50.f, GVAR_FLOAT_RANGE, GUI_CAT_GENERAL, {0.0f, 1000.f} };
GVar gvar_emission_scale_env_map{ "Env map emission scale", 2.f, GVAR_FLOAT_RANGE, GUI_CAT_GENERAL, {0.0f, 10.f} };

using namespace default_scene;
void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args)
{
	Buffer camBuf, camInstBuf;
	std::hash<std::string> hasher;
	gState.dataCache->fetch(camBuf, hasher("cam_buf"));
	gState.dataCache->fetch(camInstBuf, hasher("cam_inst_buf"));
	cmdUpdateCamera(cmdBuf, camBuf, camInstBuf, args.cameraCI);
	Buffer scalarFieldUniform = cmdGetScalarFieldUniform(cmdBuf, gState.frame->stack, gvar_medium_density_scale.val.v_float, 1.0, 1000.0);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	ComputeCmd cmd(target->getExtent2D(), shaderPath + "pt.comp", { {"FORMAT1", getGLSLFormat(target->getFormat())} });
	bindCamera(cmd, camBuf, camInstBuf);
#ifdef RAY_TRACING_SUPPORT
	bindScene(cmd, &args.sceneData);
#else
	bindMockScene(cmd);
#endif
	bindScalarField(cmd, args.mediumTexture, args.rayMarchStepSize, scalarFieldUniform);
	cmd.pushLocal();
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(args.mediumInstanceBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.pipelineDef.shaderDef.args.push_back({ "MAX_BOUNCES", args.maxDepth });
	cmd.pipelineDef.shaderDef.args.push_back({ "SAMPLE_COUNT", args.sampleCount });

	struct PushStruct
	{
		float areaLightEmissionScale;
		float envMapEmissionScale;
	} pc;
	pc.areaLightEmissionScale = gvar_emission_scale_al.val.v_float;
	pc.envMapEmissionScale = gvar_emission_scale_env_map.val.v_float;

	cmd.pushConstant(&pc, sizeof(PushStruct));

	cmd.exec(cmdBuf);
}