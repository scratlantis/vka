#include "draw_3D.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vka/core/core_common.h>
#include <vka/globals.h>
namespace vka
{
// expects position as VK_FORMAT_R32G32B32A32_SFLOAT in 0st place
void cmdShowTriangles(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, ModelData model, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout)
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
	if (clearDepth)
	{
		depthBuffer->setClearValue(ClearValue(1.0f, 0));
	}
	drawCmd.pushDepthAttachment(depthBuffer, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	drawCmd.renderArea.extent = dst->getExtent2D();
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "matrix_vec4_mult.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "assign_random_color.geom");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "vertex_color.frag");
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	drawCmd.exec(cmdBuf);
}

void cmdShowAlbedo(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, ModelData model, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout)
{
	DrawCmd     drawCmd     = DrawCmd();
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
	if (clearDepth)
	{
		depthBuffer->setClearValue(ClearValue(1.0f, 0));
	}
	drawCmd.pushDepthAttachment(depthBuffer, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	drawCmd.renderArea.extent = dst->getExtent2D();
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "matrix_vec4_mult.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_color.frag");
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	for (size_t i = 0; i < model.indexCount.size(); i++)
	{
		DrawCmd currentSurfaceDrawCmd = drawCmd;
		currentSurfaceDrawCmd.setGeometry(model.getSurface(i));
		VKA_ASSERT(currentSurfaceDrawCmd.pipelineDef.vertexAttributeDescriptions.at(0).format = VK_FORMAT_R32G32B32A32_SFLOAT);
		currentSurfaceDrawCmd.pipelineDef.vertexAttributeDescriptions = {currentSurfaceDrawCmd.pipelineDef.vertexAttributeDescriptions.at(0)};        // Only use position
		glm::vec4 color = glm::vec4(model.mtl[i].diffuse, 1.0);
		currentSurfaceDrawCmd.pushConstant(&color, sizeof(glm::vec4), VK_SHADER_STAGE_FRAGMENT_BIT);
		currentSurfaceDrawCmd.exec(cmdBuf);
	}
}

void cmdShowLines(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, ModelData model, Camera *cam, glm::mat4 objToWorld, bool clearDepth, glm::vec4 color, VkImageLayout dstLayout)
{
	DrawCmd     drawCmd     = DrawCmd();
	DrawSurface drawSurface = model.getUnifiedSurface();
	drawCmd.setGeometry(drawSurface);
	VKA_ASSERT(drawCmd.pipelineDef.vertexAttributeDescriptions.at(0).format = VK_FORMAT_R32G32B32A32_SFLOAT);
	drawCmd.pipelineDef.vertexAttributeDescriptions = {drawCmd.pipelineDef.vertexAttributeDescriptions.at(0)};        // Only use position
	glm::mat4 projectionMat                         = glm::perspective(glm::radians(60.0f), (float) dst->getExtent().width / (float) dst->getExtent().height, 0.1f, 500.0f);
	glm::mat4 vp                                    = projectionMat * cam->getViewMatrix() * objToWorld;
	drawCmd.pushDescriptor(cmdBuf, pPool, &vp, sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT);
	drawCmd.pushConstant(&color, sizeof(glm::vec4), VK_SHADER_STAGE_FRAGMENT_BIT);

	VkImageLayout finalLayout = dstLayout == VK_IMAGE_LAYOUT_UNDEFINED ? dst->getLayout() : dstLayout;
	Image depthBuffer = gState.depthBufferCache->fetch(dst->getExtent2D());
	if (clearDepth)
	{
		depthBuffer->setClearValue(ClearValue(1.0f, 0));
	}

	if (color.a == 1.0)
	{
		drawCmd.pushColorAttachment(dst, finalLayout);
		drawCmd.pushDepthAttachment(depthBuffer, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	}
	else
	{
		drawCmd.pushColorAttachment(dst, finalLayout, BlendOperation::alpha(), BlendOperation::alpha());
		drawCmd.pushDepthAttachment(depthBuffer, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	}
	drawCmd.renderArea.extent = dst->getExtent2D();
	drawCmd.pipelineDef.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "matrix_vec4_mult.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_color.frag");
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	drawCmd.exec(cmdBuf);
}

void cmdShowBoxFrame(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Camera *cam, glm::mat4 objToWorld, bool clearDepth, glm::vec4 color, VkImageLayout dstLayout)
{
	Buffer vertexBuffer = createBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	cmdWriteCopy(cmdBuf, vertexBuffer, cCubeVertecies.data(), sizeof(glm::vec3) * cCubeVertecies.size());
	Buffer indexBuffer = createBuffer(pPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	cmdWriteCopy(cmdBuf, indexBuffer, cCubeLineIndices.data(), sizeof(uint32_t) * cCubeLineIndices.size());
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	cmdShowLines<glm::vec3>(cmdBuf, pPool, dst, vertexBuffer, indexBuffer, cam, objToWorld, clearDepth, color, dstLayout);
}
void cmdShowBox(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout)
{
	Buffer vertexBuffer = createBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(glm::vec3) * cCubeVertecies.size());
	cmdWriteCopy(cmdBuf, vertexBuffer, cCubeVertecies.data(), sizeof(glm::vec3) * cCubeVertecies.size());
	Buffer indexBuffer = createBuffer(pPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	cmdWriteCopy(cmdBuf, indexBuffer, cCubeTriangleIndices.data(), sizeof(uint32_t) * cCubeTriangleIndices.size());
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	cmdShowTriangles<glm::vec3>(cmdBuf, pPool, dst, vertexBuffer, indexBuffer, cam, objToWorld, clearDepth, dstLayout);
}
}