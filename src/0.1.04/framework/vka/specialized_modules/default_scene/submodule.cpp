#include "submodule.h"
#include <glm/gtx/string_cast.hpp>
namespace vka
{
	namespace default_scene
	{
		void bindCamera(ComputeCmd &cmd, Buffer camBuf, Buffer camInstBuf)
		{
			cmd.pushSubmodule(cVkaShaderModulePath + "default_scene/pt_camera.glsl");
	        cmd.pushDescriptor(camBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	        cmd.pushDescriptor(camInstBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		}
		void cmdUpdateCamera(CmdBuffer cmdBuf, Buffer camBuf, Buffer camInstBuf, CameraCI ci)
		{
	        GLSLCamera cam{};
	        cam.projection    = glm::perspective(glm::radians(ci.yFovDeg), (float) ci.extent.width / (float) ci.extent.height, ci.zNear, ci.zFar);
	        cam.invProjection = glm::inverse(cam.projection);
	        cam.width = ci.extent.width;
	        cam.height = ci.extent.height;
			camBuf->addUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	        cmdWriteCopy(cmdBuf, camBuf, &cam, sizeof(GLSLCamera));

			GLSLCameraInstance camInst{};
	        camInst.view = glm::lookAt(ci.pos, ci.pos + glm::normalize(ci.frontDir), glm::normalize(ci.upDir));
	        camInst.invView = glm::inverse(camInst.view);
	        camInst.seed = ci.seed;
			camInstBuf->addUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
			cmdWriteCopy(cmdBuf, camInstBuf, &camInst, sizeof(GLSLCameraInstance));
		}
        void bindScene(ComputeCmd &cmd, const pbr::USceneData *pScene)
        {
	        cmd.pushSubmodule(cVkaShaderModulePath + "default_scene/pt_uscene.glsl");
	        cmd.pushDescriptor(pScene->vertexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(pScene->indexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(pScene->modelOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(pScene->surfaceOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(pScene->materialBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(pScene->areaLightBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(pScene->tlas);
	        cmd.pushDescriptor(pScene->instanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(pScene->instanceOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	        cmd.pushDescriptor(SamplerDefinition());
	        cmd.pushDescriptor(pScene->textures, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	        cmd.pushDescriptor(pScene->envMap, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	        cmd.pushDescriptor(pScene->envMapPdfBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	        cmd.pipelineDef.shaderDef.args.push_back({"ENVMAP_PDF_BINS_X", pScene->envMapSubdivisions.x});
	        cmd.pipelineDef.shaderDef.args.push_back({"ENVMAP_PDF_BINS_Y", pScene->envMapSubdivisions.y});
	        cmd.pipelineDef.shaderDef.args.push_back({"AREA_LIGHT_COUNT", pScene->areaLightCount});
        }
        void bindMockScene(ComputeCmd &cmd)
        {
	        cmd.pushSubmodule(cVkaShaderModulePath + "mock/pt_uscene_mock.glsl");
        }
        void bindScalarField(ComputeCmd &cmd, Image scalarField, float rayMarchStepSize)
		{
	        cmd.pushSubmodule(cVkaShaderModulePath + "pt_scalar_field.glsl");
			cmd.pushDescriptor(scalarField, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
			cmd.pipelineDef.shaderDef.args.push_back({"RAY_MARCH_STEP_SIZE", std::to_string(rayMarchStepSize)});
	        VKA_ASSERT(scalarField->getExtent().width == scalarField->getExtent().height && scalarField->getExtent().depth == scalarField->getExtent().height);
	        cmd.pipelineDef.shaderDef.args.push_back({"VOLUME_RESOLUTION", scalarField->getExtent().width});
		}
	}
}