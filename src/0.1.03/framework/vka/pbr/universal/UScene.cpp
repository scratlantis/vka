#include "UScene.h"
#include <vka/globals.h>
#include <vka/specialized_utility/draw_2D.h>

namespace vka
{
namespace pbr
{
void USceneBuilderBase::loadEnvMap(const ImagePdfKey &key)
{
	// Load env map & pdf
	// Todo
}

void USceneBuilderBase::addModel(CmdBuffer cmdBuf, std::string path, glm::mat4 transform, uint32_t loadFlags)
{
	addModelInternal(cmdBuf, gState.modelCache, path, loadFlags | MODEL_LOAD_FLAG_CREATE_ACCELERATION_STRUCTURE | MODEL_LOAD_FLAG_COPYABLE);
	transformList.push_back(transform);
}

USceneData USceneBuilderBase::create(CmdBuffer cmdBuf, IResourcePool *pPool, uint32_t sceneLoadFlags = 0)
{
	cmdBarrier(cmdBuf,
	           VK_PIPELINE_STAGE_TRANSFER_BIT,
	           VK_PIPELINE_STAGE_TRANSFER_BIT,
	           VK_ACCESS_TRANSFER_WRITE_BIT,
	           VK_ACCESS_TRANSFER_READ_BIT);
	uint32_t additinalVertexBufferUsage = 0;
	uint32_t additinalIndexBufferUsage  = 0;

	if (sceneLoadFlags & SCENE_LOAD_FLAG_ALLOW_RASTERIZATION)
	{
		additinalVertexBufferUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		additinalIndexBufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}
	USceneData sceneData{};
	// May be to set false bellow
	sceneData.info.useAreaLight = true;
	sceneData.info.useEnvMap = true;
	sceneData.info.useTextures = true;
	// Create buffers
	VkDeviceSize totalVertexBufferSize  = 0;
	VkDeviceSize totalIndexBufferOffset = 0;
	uint32_t     vertexStride           = modelList[0].vertexLayout.stride;
	for (auto &model : modelList)
	{
		totalVertexBufferSize += model.vertexBuffer->getSize();
		totalIndexBufferOffset += model.indexBuffer->getSize();
	}
	sceneData.vertexBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | additinalVertexBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY, totalVertexBufferSize);
	sceneData.indexBuffer  = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | additinalIndexBufferUsage, VMA_MEMORY_USAGE_GPU_ONLY, totalIndexBufferOffset);

	// Copy vertex & index data and store offsets
	VkDeviceSize                   vertexOffsett = 0;
	VkDeviceSize                   indexOffsett  = 0;
	uint32_t                       surfaceCount = 0;
	std::vector<ModelDataOffset>   modelDataOffset(modelList.size());
	std::vector<AreaLight>         areaLights;
	std::vector<uint32_t>		   indexCounts;
	for (size_t i = 0; i < modelList.size(); i++)
	{
		cmdCopyBuffer(cmdBuf, modelList[i].vertexBuffer, sceneData.vertexBuffer->getSubBuffer({vertexOffsett, modelList[i].vertexBuffer->getSize()}));
		cmdCopyBuffer(cmdBuf, modelList[i].indexBuffer, sceneData.indexBuffer->getSubBuffer({indexOffsett, modelList[i].indexBuffer->getSize()}));

		modelDataOffset[i].firstVertex   = vertexOffsett / vertexStride;
		modelDataOffset[i].firstSurface = surfaceCount;
		indexCounts.insert(indexCounts.end(), modelList[i].indexCount.begin(), modelList[i].indexCount.end());

		areaLights.insert(areaLights.end(), modelList[i].lights.begin(), modelList[i].lights.end());

		surfaceCount += modelList[i].indexCount.size();
		vertexOffsett += modelList[i].vertexBuffer->getSize();
		indexOffsett += modelList[i].indexBuffer->getSize();
	}

	// Per model offesets
	sceneData.modelOffsetBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	cmdWriteCopy(cmdBuf, sceneData.modelOffsetBuffer, modelDataOffset.data(), modelDataOffset.size() * sizeof(ModelDataOffset));

	std::vector<uint32_t> firstSurfaceIndex(indexCounts.size());
	uint32_t sum = 0;
	for (size_t i = 0; i < indexCounts.size(); i++)
	{
		firstSurfaceIndex[i] = sum;
		sum += indexCounts[i];
	}

	// Per surface offset
	sceneData.surfaceOffsetBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	cmdWriteCopy(cmdBuf, sceneData.surfaceOffsetBuffer, firstSurfaceIndex.data(), firstSurfaceIndex.size() * sizeof(uint32_t));


	if (!areaLights.empty())
	{
		sceneData.areaLightBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		cmdWriteCopy(cmdBuf, sceneData.areaLightBuffer, areaLights.data(), areaLights.size() * sizeof(AreaLight));
	}
	else
	{
		sceneData.info.useAreaLight = false;
		sceneData.areaLightBuffer    = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(AreaLight));
	}

	sceneData.materialBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	loadMaterials(cmdBuf, sceneData.materialBuffer);

	sceneData.textures.resize(textureIndexMap.size());
	for (auto &it : textureIndexMap)
	{
		sceneData.textures[it.second] = gState.textureCache->fetch(cmdBuf, it.first, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	if (sceneData.textures.empty())
	{
		sceneData.info.useTextures = false;
		sceneData.textures.push_back(cmdCreateDummyTexture(cmdBuf, pPool));
	}
	sceneData.tlas = createTopLevelAS(pPool, modelList.size());

	if (!envMapName.empty())
	{
		sceneData.envMap          = gState.textureCache->fetch(cmdBuf, envMapName, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		sceneData.envMapPdfBuffer = pdfCache->fetch(cmdBuf, {envMapName, {16, 16}});
	}
	else
	{
		sceneData.info.useEnvMap  = false;
		sceneData.envMap          = cmdCreateDummyTexture(cmdBuf, pPool);
		sceneData.envMapPdfBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 16);
	}

	return sceneData;
}
Buffer USceneBuilderBase::uploadInstanceData(CmdBuffer cmdBuf, IResourcePool *pPool)
{
	Buffer                                          buf = createBuffer(pPool, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY);
	std::vector<VkAccelerationStructureInstanceKHR> instances(transformList.size());
	for (uint32_t i = 0; i < instances.size(); i++)
	{
		instances[i].transform                              = glmToVk(transformList[i]);
		instances[i].instanceCustomIndex                    = i;
		instances[i].mask                                   = 0xFF;
		instances[i].instanceShaderBindingTableRecordOffset = 0;
		instances[i].flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instances[i].accelerationStructureReference         = modelList[i].blas->getDeviceAddress();
	}
	cmdWriteCopy(cmdBuf, buf, instances.data(), instances.size() * sizeof(VkAccelerationStructureInstanceKHR));
	return buf;
}
void USceneBuilderBase::setTransform(std::string path, glm::mat4 transform)
{
	uint32_t index       = indexMap.at(path);
	transformList[index] = transform;
}
void USceneBuilderBase::reset()
{
	textureIndexMap.clear();
	modelList.clear();
	transformList.clear();
	indexMap.clear();
	envMapName = "";
}
void USceneData::build(CmdBuffer cmdBuf, Buffer instanceBuffer)
{
	cmdBarrier(cmdBuf,
	           VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
	           VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
	           VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
	           VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR);
	cmdBuildAccelerationStructure(cmdBuf, tlas, instanceBuffer, createStagingBuffer());
}
}        // namespace pbr
}        // namespace vka
