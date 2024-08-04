#include "ModelCache.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <vka/core/core_utility/general_commands.h>
#include <vka/core/core_utility/buffer_utility.h>
#include <vka/core/core_utility/acceleration_structure_utility.h>
namespace vka
{

bool loadObj(std::string path, std::vector<ObjVertex> &vertexList, std::vector<Index> &indexList, std::vector<uint32_t> &indexOffsets, uint32_t &totalIndexCount)
{
	tinyobj::attrib_t                vertexAttributes;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      errorString;
	std::string                      warningString;
	bool                             success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warningString, &errorString, path.c_str());

	if (!success)
	{
		std::cerr << "Failed to load model: " << path << std::endl;
		DEBUG_BREAK
		return false;
	}
	std::unordered_map<ObjVertex, uint32_t> vertexMap;
	vertexMap.reserve(vertexAttributes.vertices.size());
	vertexList.reserve(vertexAttributes.vertices.size());
	totalIndexCount       = 0;
	uint32_t surfaceCount = 0;
	for (auto &shape : shapes)
	{
		totalIndexCount += shape.mesh.indices.size();
		surfaceCount++;
	}
	indexList.reserve(totalIndexCount);
	indexOffsets.reserve(surfaceCount);
	uint64_t cnt = 0;
	for (auto &shape : shapes)
	{
		indexOffsets.push_back(indexList.size());
		for (auto &index : shape.mesh.indices)
		{
			ObjVertex objVertex{};
			objVertex.v =
			    glm::vec3(
			        vertexAttributes.vertices[index.vertex_index * 3],
			        vertexAttributes.vertices[index.vertex_index * 3 + 1],
			        vertexAttributes.vertices[index.vertex_index * 3 + 2]);
			if (index.texcoord_index >= 0)
				objVertex.vt =
				    glm::vec2(
				        vertexAttributes.texcoords[index.texcoord_index * 2],
				        vertexAttributes.texcoords[index.texcoord_index * 2 + 1]);
			if (index.normal_index >= 0)
				objVertex.vn =
				    glm::vec3(
				        vertexAttributes.normals[index.normal_index * 3],
				        vertexAttributes.normals[index.normal_index * 3 + 1],
				        vertexAttributes.normals[index.normal_index * 3 + 2]);
			auto it = vertexMap.insert({objVertex, cnt});
			// New vertex
			if (it.second)
			{
				vertexList.push_back(objVertex);
				indexList.push_back(cnt++);
			}
			// Vertex already exists
			else
			{
				indexList.push_back(it.first->second);
			}
		}
	}
	vertexList.shrink_to_fit();
	indexList.shrink_to_fit();
	return true;
}

void ModelCache::clear()
{
	for (auto &model : map)
	{
		if (model.second.vertexBuffer)
			model.second.vertexBuffer->garbageCollect();
		if (model.second.indexBuffer)
			model.second.indexBuffer->garbageCollect();
	}
	map.clear();
}

ModelData ModelCache::fetch(CmdBuffer cmdBuf, std::string path, void (*parse)(Buffer vertexBuffer, VertexDataLayout &vertexLayout, const std::vector<ObjVertex> &vertexList), uint32_t loadFlags)
{
	ModelKey key{path, parse, loadFlags};
	auto     it = map.find(key);
	if (it == map.end())
	{
		ModelData modelData{};
		VkBufferUsageFlags additionalBufferUsageFlags = 0;
		if (loadFlags & MODEL_LOAD_FLAG_CREATE_ACCELERATION_STRUCTURE)
		{
			additionalBufferUsageFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; 
		}
		modelData.vertexBuffer          = createBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsageFlags | additionalBufferUsageFlags);
		modelData.indexBuffer           = createBuffer(pPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsageFlags | additionalBufferUsageFlags);
		std::string            fullPath = modelPath + path;
		std::vector<ObjVertex> vertexList;
		std::vector<Index>     indexList;
		if (loadObj(fullPath, vertexList, indexList, modelData.indexOffsets, modelData.indexCount))
		{
			printVka(("Loading model: " + path).c_str());
			map.insert({key, modelData});
			parse(modelData.vertexBuffer, modelData.vertexLayout, vertexList);
			write(modelData.indexBuffer, indexList.data(), indexList.size()*sizeof(Index));
			cmdUpload(cmdBuf, modelData.vertexBuffer);
			cmdUpload(cmdBuf, modelData.indexBuffer);

			if (loadFlags & MODEL_LOAD_FLAG_CREATE_ACCELERATION_STRUCTURE)
			{
				std::vector<VkAccelerationStructureGeometryKHR>       geometry;
				std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange;
				for (uint32_t i = 0; i < modelData.indexOffsets.size(); i++)
				{
					VkAccelerationStructureGeometryTrianglesDataKHR trianglesKHR{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR};
					trianglesKHR.vertexFormat             = VK_FORMAT_R32G32B32_SFLOAT;
					trianglesKHR.vertexData.deviceAddress = modelData.vertexBuffer->getDeviceAddress();
					trianglesKHR.vertexStride             = modelData.vertexBuffer->getSize() / vertexList.size();
					trianglesKHR.indexType                = VK_INDEX_TYPE_UINT32;
					trianglesKHR.indexData.deviceAddress  = modelData.indexBuffer->getDeviceAddress();
					VkAccelerationStructureGeometryKHR geometryKHR{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
					geometryKHR.geometryType       = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
					geometryKHR.geometry.triangles = trianglesKHR;
					geometryKHR.flags              = (loadFlags & MODEL_LOAD_FLAG_IS_OPAQUE) ? VK_GEOMETRY_OPAQUE_BIT_KHR : 0;
					geometry.push_back(geometryKHR);

					VkAccelerationStructureBuildRangeInfoKHR rangeKHR{};
					rangeKHR.primitiveCount  = indexList.size() / 3;
					rangeKHR.primitiveOffset = 0;
					rangeKHR.firstVertex     = 0;
					rangeKHR.transformOffset = 0;
					buildRange.push_back(rangeKHR);
				}
				modelData.blas = createBottomLevelAS(pPool, geometry, buildRange);
				cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR);
				cmdBuildAccelerationStructure(cmdBuf, modelData.blas, createStagingBuffer());
			}

		}
		return modelData;
	}
	return it->second;
}

}        // namespace vka
