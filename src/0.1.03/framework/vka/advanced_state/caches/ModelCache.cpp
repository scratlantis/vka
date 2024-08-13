#include "ModelCache.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <vka/core/core_utility/general_commands.h>
#include <vka/core/core_utility/buffer_utility.h>
#include <vka/core/core_utility/acceleration_structure_utility.h>
namespace vka
{

bool ModelCache::loadObj(std::string path, std::vector<ObjVertex> &vertexList, std::vector<Index> &indexList, std::vector<uint32_t> &indexCountList, std::vector<WavefrontMaterial> &surfaceList) const
{
	tinyobj::attrib_t                vertexAttributes;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      errorString;
	std::string                      warningString;

	// Parse obj and mtl files
	std::string mtldir = path.substr(0, path.find_last_of("/"));
	bool        success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warningString, &errorString, path.c_str(), mtldir.c_str());
	if (!success)
	{
		std::cerr << "Failed to load model: " << path << std::endl;
		DEBUG_BREAK
		return false;
	}

	// Convert to internal format:
	// Obj
	std::unordered_map<ObjVertex, uint32_t> vertexMap;
	vertexMap.reserve(vertexAttributes.vertices.size());
	vertexList.reserve(vertexAttributes.vertices.size());
	indexCountList.reserve(shapes.size());
	
	uint64_t totalIndexCount  = 0;
	for (auto &shape : shapes)
	{
		totalIndexCount += shape.mesh.indices.size();
		indexCountList.push_back(shape.mesh.indices.size());
	}
	indexList.reserve(totalIndexCount);
	uint64_t cnt = 0;
	for (auto &shape : shapes)
	{
		for (auto &index : shape.mesh.indices)
		{
			ObjVertex objVertex{};
			objVertex.v =
			    glm::vec3(
			        vertexAttributes.vertices[index.vertex_index * 3],
			        -vertexAttributes.vertices[index.vertex_index * 3 + 1], // inv y coord
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
				        -vertexAttributes.normals[index.normal_index * 3 + 1], // inv y coord
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

	// Mtl
	surfaceList.reserve(materials.size());
	for (auto &shape : shapes)
	{
		VKA_ASSERT(!shape.mesh.material_ids.empty());
		uint32_t matId = shape.mesh.material_ids[0];
		
		IF_VALIDATION(
		bool multiMat = false;
		for (auto &id : shape.mesh.material_ids) {
			if (id != matId)
			{
				multiMat = true;
				break;
			}
		})
		VKA_ASSERT(!multiMat);

		WavefrontMaterial surface{};
		if (matId != -1)
		{
			auto &mat = materials[matId];
			surface.name             = mat.name;
			surface.ambient          = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
			surface.ambientMap       = mat.ambient_texname;
			surface.diffuse          = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
			surface.diffuseMap       = mat.diffuse_texname;
			surface.specular         = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
			surface.specularMap      = mat.specular_texname;
			surface.specularExponent = mat.shininess;
			surface.emission         = glm::vec3(mat.emission[0], mat.emission[1], mat.emission[2]);
			surface.emissionMap      = mat.emissive_texname;
			surface.roughness        = mat.roughness;
			surface.roughnessMap     = mat.roughness_texname;
			surface.metallic         = mat.metallic;
			surface.metallicMap      = mat.metallic_texname;
			surface.dissolve         = mat.dissolve;
			surface.ior              = mat.ior;
			surface.normalMap        = mat.normal_texname;
		}
		surfaceList.push_back(surface);
	}

	return true;
}

BLAS ModelCache::buildAccelerationStructure(CmdBuffer cmdBuf, const ModelData &modelData, uint32_t loadFlags) const
{
	std::vector<VkAccelerationStructureGeometryKHR>       geometry;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange;
	uint32_t                                              offset = 0;
	for (uint32_t i = 0; i < modelData.indexCount.size(); i++)
	{
		VkAccelerationStructureGeometryTrianglesDataKHR trianglesKHR{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR};
		trianglesKHR.vertexFormat             = modelData.vertexLayout.formats[0];        // We hereby assume that the vertex position is the first attribute
		trianglesKHR.vertexData.deviceAddress = modelData.vertexBuffer->getDeviceAddress();
		trianglesKHR.vertexStride             = modelData.vertexLayout.stride;
		trianglesKHR.indexType                = VK_INDEX_TYPE_UINT32;
		trianglesKHR.indexData.deviceAddress  = modelData.indexBuffer->getDeviceAddress();
		VkAccelerationStructureGeometryKHR geometryKHR{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
		geometryKHR.geometryType       = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometryKHR.geometry.triangles = trianglesKHR;
		geometryKHR.flags              = (loadFlags & MODEL_LOAD_FLAG_IS_OPAQUE) ? VK_GEOMETRY_OPAQUE_BIT_KHR : 0;
		geometry.push_back(geometryKHR);

		VkAccelerationStructureBuildRangeInfoKHR rangeKHR{};
		rangeKHR.primitiveCount  = modelData.indexCount[i] / 3;
		rangeKHR.primitiveOffset = offset * sizeof(Index);
		rangeKHR.firstVertex     = 0;
		rangeKHR.transformOffset = 0;
		buildRange.push_back(rangeKHR);

		offset += modelData.indexCount[i];
	}
	BLAS blas = createBottomLevelAS(pPool, geometry, buildRange);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR);
	cmdBuildAccelerationStructure(cmdBuf, blas, createStagingBuffer());
	return blas;
}

void ModelCache::findAreaLights(std::vector<AreaLight> &lightList, const std::vector<ObjVertex> &vertexList, const std::vector<Index> &indexList, const std::vector<uint32_t> &indexCountList, const std::vector<WavefrontMaterial> &surfaceList) const
{
	// Area Lights for importance sampling
	uint32_t indexOffset = 0;
	for (size_t i = 0; i < surfaceList.size(); i++)
	{
		const WavefrontMaterial &surface  = surfaceList[i];
		float              emission = surface.emission.x + surface.emission.y + surface.emission.z;
		if (emission > 0)
		{
			for (size_t j = indexOffset; j < indexOffset + indexCountList[i]; j += 3)
			{
				AreaLight light{};
				light.v0       = vertexList[indexList[j]].v;
				light.v1       = vertexList[indexList[j + 1]].v;
				light.v2       = vertexList[indexList[j + 2]].v;
				glm::vec3 normal = glm::normalize(glm::cross(light.v1 - light.v0, light.v2 - light.v0));
				light.center     = (light.v0 + light.v1 + light.v2) / 3.0f;
				light.normal     = glm::normalize(normal);
				light.importance = glm::length(emission * normal);
				lightList.push_back(light);
			}
		}

		indexOffset += indexCountList[i];
	}
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

}        // namespace vka
