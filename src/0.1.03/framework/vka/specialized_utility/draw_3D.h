#pragma once
#include <vka/advanced_utility/complex_commands.h>
#include <vka/controller/camera/Camera.h>
namespace vka
{
void cmdShowTriangles(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, ModelData model, Camera *cam, glm::mat4 objToWorld, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED);
template<class Vertex>
void cmdShowTriangles(CmdBuffer cmdBuf, IResourcePool *pPool, Image dst, Buffer vertexBuffer, Buffer indexBuffer, Camera *cam, glm::mat4 objToWorld, VkImageLayout dstLayout = VK_IMAGE_LAYOUT_UNDEFINED)
{
	ModelData modelData{};
	modelData.vertexBuffer = vertexBuffer;
	modelData.indexBuffer  = indexBuffer;
	modelData.vertexLayout = vertex_type<Vertex>().data_layout();
	modelData.indexCount = {static_cast<uint32_t>(indexBuffer->getSize()) / sizeof(Index)};
	cmdShowTriangles(cmdBuf, pPool, dst, modelData, cam, objToWorld, dstLayout);
}
} // namespace vka