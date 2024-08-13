#include "draw_3D.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vka/core/core_common.h>
#include <vka/globals.h>
namespace vka
{
// expects position as VK_FORMAT_R32G32B32A32_SFLOAT in 0st place
void cmdShowTriangles(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, ModelData model, Camera *cam, glm::mat4 objToWorld, VkImageLayout dstLayout)
{
	DrawCmd drawCmd = DrawCmd();
	DrawSurface drawSurface = model.getUnifiedSurface();
	drawCmd.setGeometry(drawSurface);
	VKA_ASSERT(drawCmd.pipelineDef.vertexAttributeDescriptions.at(0).format = VK_FORMAT_R32G32B32A32_SFLOAT);
	drawCmd.pipelineDef.vertexAttributeDescriptions = {drawCmd.pipelineDef.vertexAttributeDescriptions.at(0)};        // Only use position
	glm::mat4 projectionMat                         = glm::perspective(glm::radians(60.0f), (float) dst->getExtent().width / (float) dst->getExtent().height, 0.1f, 500.0f);
	glm::mat4 vp                                    = projectionMat * cam->getViewMatrix() * objToWorld;
	drawCmd.pushDescriptor(cmdBuf, pPool, &vp, sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT);
	if (dstLayout == VK_IMAGE_LAYOUT_UNDEFINED)
	{
		drawCmd.pushColorAttachment(dst);
	}
	else
	{
		drawCmd.pushColorAttachment(dst, dstLayout);
	}
	Image depthBuffer = gState.depthBufferCache->fetch(dst->getExtent2D());
	depthBuffer->setClearValue(ClearValue(1.0f, 0));
	drawCmd.pushDepthAttachment(depthBuffer, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	drawCmd.renderArea.extent = dst->getExtent2D();
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "matrix_vec4_mult.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "assign_random_color.geom");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "vertex_color.frag");
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	drawCmd.exec(cmdBuf);
}
}