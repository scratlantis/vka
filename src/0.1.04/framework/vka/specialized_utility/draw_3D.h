#pragma once
#include <vka/advanced_utility/complex_commands.h>
#include <vka/controller/camera/Camera.h>
namespace vka
{
void cmdShowTriangles(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, ModelData model, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED);
void cmdShowAlbedo(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, ModelData model, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED);
template<class Vertex>
void cmdShowTriangles(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Buffer vertexBuffer, Buffer indexBuffer, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED)
{
	ModelData modelData{};
	modelData.vertexBuffer = vertexBuffer;
	modelData.indexBuffer  = indexBuffer;
	modelData.vertexLayout = vertex_type<Vertex>().data_layout();
	if (indexBuffer)
	{
		modelData.indexCount = {static_cast<uint32_t>(indexBuffer->getSize()) / sizeof(Index)};
	}
	else
	{
		modelData.indexCount = {static_cast<uint32_t>(vertexBuffer->getSize()) / modelData.vertexLayout.stride};
	}
	cmdShowTriangles(cmdBuf, pPool, dst, modelData, cam, objToWorld, clearDepth, dstLayout);
}



void cmdShowLines(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, ModelData model, Camera *cam, glm::mat4 objToWorld, bool clearDepth, glm::vec4 color, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED);
template <class Vertex>
void cmdShowLines(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Buffer vertexBuffer, Buffer indexBuffer, Camera *cam, glm::mat4 objToWorld, bool clearDepth, glm::vec4 color, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED)
{
	ModelData modelData{};
	modelData.vertexBuffer = vertexBuffer;
	modelData.indexBuffer  = indexBuffer;
	modelData.vertexLayout = vertex_type<Vertex>().data_layout();
	if (indexBuffer)
	{
		modelData.indexCount   = {static_cast<uint32_t>(indexBuffer->getSize()) / sizeof(Index)};
	}
	else
	{
		modelData.indexCount = {static_cast<uint32_t>(vertexBuffer->getSize()) / modelData.vertexLayout.stride};
	}
	cmdShowLines(cmdBuf, pPool, dst, modelData, cam, objToWorld, clearDepth, color, dstLayout);
}
void cmdShowBoxFrame(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Camera *cam, glm::mat4 objToWorld, bool clearDepth, glm::vec4 color, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED);
void cmdShowBox(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Camera *cam, glm::mat4 objToWorld, bool clearDepth, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED);
} // namespace vka