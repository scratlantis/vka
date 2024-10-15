#include "UScene.h"
#include <vka/globals.h>
#include <vka/specialized_utility/draw_2D.h>
#include <vka/specialized_utility/compute.h>

namespace vka
{
namespace pbr
{
void USceneBuilderBase::loadEnvMap(std::string name, glm::uvec2 subdivisions)
{
	envMapName = name;
	envMapSubdivisions = subdivisions;
}

void USceneBuilderBase::addModel(CmdBuffer cmdBuf, std::string path, void *instanceData, uint32_t instanceCount, uint32_t loadFlags)
{
	addModelInternal(cmdBuf, gState.modelCache, path, instanceData, instanceCount, loadFlags | MODEL_LOAD_FLAG_CREATE_ACCELERATION_STRUCTURE | MODEL_LOAD_FLAG_COPYABLE);
}

void USceneBuilderBase::addModel(CmdBuffer cmdBuf, std::string path, Buffer instanceBuffer, uint32_t instanceCount, uint32_t loadFlags)
{
	addModelInternal(cmdBuf, gState.modelCache, path, instanceBuffer, instanceCount, loadFlags | MODEL_LOAD_FLAG_CREATE_ACCELERATION_STRUCTURE | MODEL_LOAD_FLAG_COPYABLE);
}


USceneData USceneBuilderBase::create(CmdBuffer cmdBuf, IResourcePool *pPool, uint32_t sceneLoadFlags)
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


		for (size_t k = 0; k < getInstanceCount(i); k++)
		{
			std::vector<AreaLight> lights = modelList[i].lights;
			for (size_t j = 0; j < lights.size(); j++)
			{
				lights[j].instanceIndex = getInstanceOffset(i) + k;//civ
			}

			areaLights.insert(areaLights.end(), lights.begin(), lights.end());
		}

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

	sceneData.areaLightCount = areaLights.size();
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
	//sceneData.tlas = createTopLevelAS(pPool, modelList.size());
	sceneData.tlas = createTopLevelAS(pPool, getTotalInstanceCount());

	if (!envMapName.empty())
	{
		sceneData.envMap             = gState.textureCache->fetch(cmdBuf, envMapName, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		hash_t        key = envMapName HASHC envMapSubdivisions.x HASHC envMapSubdivisions.y;
		if (!gState.dataCache->fetch(sceneData.envMapPdfBuffer, key))
		{
			cmdComputeImgPdf(cmdBuf, sceneData.envMap, sceneData.envMapPdfBuffer, envMapSubdivisions.x, envMapSubdivisions.y);
		}
		sceneData.envMapSubdivisions = envMapSubdivisions;
	}
	else
	{
		sceneData.info.useEnvMap  = false;
		sceneData.envMap          = cmdCreateDummyTexture(cmdBuf, pPool);
		sceneData.envMapPdfBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 16);
		sceneData.envMapSubdivisions = {1, 1};
	}
	sceneData.isInitialized = true;
	return sceneData;
}

void USceneData::build(CmdBuffer cmdBuf, USceneInstanceData instanceData)
{
	cmdBarrier(cmdBuf,
	           VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
	           VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
	           VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
	           VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR);
	cmdBuildAccelerationStructure(cmdBuf, tlas, instanceData.tlasInstanceBuffer, createStagingBuffer());
	this->instanceBuffer = instanceData.instanceBuffer;
	this->instanceOffsetBuffer = instanceData.modelIndexBuffer;
}
}        // namespace pbr
}        // namespace vka
