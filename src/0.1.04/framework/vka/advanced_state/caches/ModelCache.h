#pragma once
#include <vka/core/stateless/utility/macros.h>
#include <vka/core/resources/unique/Buffer.h>
#include <vka/core/core_utility/types.h>
#include <unordered_map>
namespace vka
{
typedef uint32_t Index;

struct ObjVertex
{
	glm::vec3 v;
	glm::vec2 vt;
	glm::vec3 vn;

	ObjVertex(glm::vec3 v, glm::vec2 vt, glm::vec3 vn) :
	    v(v), vt(vt), vn(vn)
	{}
	ObjVertex() :
	    v(0), vt(0), vn(0)
	{}
	bool operator==(const ObjVertex &other) const
	{
		return v == other.v && vt == other.vt && vn == other.vn;
	}
	vka::hash_t hash() const
	{
		return std::hash<glm::vec3>()(v) ^ std::hash<glm::vec2>()(vt) ^ std::hash<glm::vec3>()(vn);
	}
};

struct WavefrontMaterial
{
	std::string name;

	// Material parameters
	glm::vec3   ambient;
	std::string ambientMap;

	glm::vec3   diffuse;
	std::string diffuseMap;

	glm::vec3   specular;
	std::string specularMap;
	float       specularExponent;

	glm::vec3   emission;
	std::string emissionMap;

	float       roughness;
	std::string roughnessMap;

	float       metallic;
	std::string metallicMap;

	// Transparency
	float dissolve;        // 1.0 = opaque, 0.0 = fully transparent
	float ior;             // index of refraction

	// Geometric Properties
	std::string normalMap;
};

// Shader struct, respect alignment rules (https://registry.khronos.org/OpenGL/extensions/ARB/ARB_uniform_buffer_object.txt)
struct AreaLight
{
	// NEE select light source
	glm::vec3 center;
	uint32_t  instanceIndex;

	glm::vec3 normal;
	uint32_t  padding1[1];

	glm::vec3 v0; // Vertices
	uint32_t  padding2[1];

	glm::vec3 v1; // Vertices
	uint32_t  padding3[1];

	glm::vec3 v2; // Vertices
	float intensity; // Area * intensity

};
static_assert(offsetof(AreaLight, normal) == 1 * 16, "Offset is not correct");
static_assert(offsetof(AreaLight, v0) == 2 * 16, "Offset is not correct");
static_assert(offsetof(AreaLight, v1) == 3 * 16, "Offset is not correct");
static_assert(offsetof(AreaLight, v2) == 4 * 16, "Offset is not correct");
static_assert(offsetof(AreaLight, intensity) == 4 * 16 + 3 * 4, "Offset is not correct");
static_assert(sizeof(AreaLight) == 5 * 16, "Size is not correct");


struct VertexDataLayout
{
	std::vector<VkFormat> formats;
	std::vector<uint32_t> offsets;
	uint32_t              stride;

	bool operator==(const VertexDataLayout &other) const
	{
		return cmpVector(formats, other.formats) && cmpVector(offsets, other.offsets) && stride == other.stride;
	}

	vka::hash_t hash() const
	{
		return hashVector(formats) HASHC hashVector(offsets) HASHC stride;
	}
};




struct DrawSurface
{
	Buffer           vertexBuffer;
	Buffer           indexBuffer;
	uint32_t         offset;
	uint32_t         count;
	VertexDataLayout vertexLayout;

	static DrawSurface screenFillingTriangle()
	{
		return {nullptr, nullptr, 0, 3, {}};
	}
};

enum ModelLoadFlagBits
{
	MODEL_LOAD_FLAG_CREATE_ACCELERATION_STRUCTURE = 1 << 0,
	MODEL_LOAD_FLAG_IS_OPAQUE = 1 << 1,
	MODEL_LOAD_FLAG_COPYABLE = 1 << 2
};

struct ModelData
{
	Buffer                         vertexBuffer = nullptr;
	Buffer                         indexBuffer  = nullptr;
	BLAS                           blas         = nullptr;
	VertexDataLayout               vertexLayout;
	std::vector<uint32_t>          indexCount;
	std::vector<WavefrontMaterial> mtl;
	std::vector<AreaLight>         lights;
	


	bool operator==(const ModelData &other) const
	{
		return vertexBuffer == other.vertexBuffer && indexBuffer == other.indexBuffer && cmpVector(indexCount, other.indexCount) && blas == other.blas;
	};
	vka::hash_t hash() const
	{
		return vertexBuffer HASHC indexBuffer HASHC hashVector(indexCount) HASHC blas;
	}

	DrawSurface getSurface(uint32_t idx) const
	{
		uint32_t offset = 0;
		for (size_t i = 0; i < idx; i++)
		{
			offset += indexCount[i];
		}
		DrawSurface surface{};
		surface.vertexBuffer = vertexBuffer;
		surface.indexBuffer = indexBuffer;
		surface.offset = offset;
		surface.count = indexCount[idx];
		surface.vertexLayout = vertexLayout;
		return surface;
	}

	DrawSurface getUnifiedSurface() const
	{
		DrawSurface surface{};
		surface.vertexBuffer = vertexBuffer;
		surface.indexBuffer = indexBuffer;
		surface.offset = 0;
		surface.count = 0;
		for (auto &cnt : indexCount)
		{
			surface.count += cnt;
		}
		surface.vertexLayout = vertexLayout;
		return surface;
	}
};

struct ModelKey
{
	std::string      path;
	std::string      type;
	uint32_t         loadFlags;

	bool operator==(const ModelKey &other) const
	{
		return path == other.path && type == type && loadFlags == other.loadFlags;
	}

	vka::hash_t hash() const
	{
		return std::hash<std::string>()(path) ^ std::hash<std::string>()(type) ^ loadFlags;
	}
};
}		// namespace vka
DECLARE_HASH(vka::ModelData, hash);
DECLARE_HASH(vka::ModelKey, hash);
DECLARE_HASH(vka::ObjVertex, hash);

namespace vka
{
template <typename Vertex>
struct vertex_type;

template <>
struct vertex_type<glm::vec3>
{
	VertexDataLayout data_layout()
	{
		VertexDataLayout layout{};
		layout.formats =
		    {
		        VK_FORMAT_R32G32B32_SFLOAT};
		layout.offsets =
		    {
		        0};
		layout.stride = sizeof(glm::vec3);
		return layout;
	}
};




class ModelCache
{
  private:
	std::unordered_map<ModelKey, ModelData> map;
	std::string                             modelPath;
	IResourcePool                          *pPool;
	VkBufferUsageFlags                      bufferUsageFlags;

	bool loadObj(std::string path, std::vector<ObjVertex> &vertexList, std::vector<Index> &indexList, std::vector<uint32_t> &indexCountList, std::vector<WavefrontMaterial> &surfaceList) const;
	BLAS buildAccelerationStructure(CmdBuffer cmdBuf, const ModelData &modelData, uint32_t loadFlags) const;
	void findAreaLights(std::vector<AreaLight> &lightList, const std::vector<ObjVertex> &vertexList, const std::vector<Index> &indexList, const std::vector<uint32_t> &indexCountList, const std::vector<WavefrontMaterial> &surfaceList) const;

  public:
	ModelCache(IResourcePool *pPool, std::string modelPath, VkBufferUsageFlags bufferUsageFlags) :
	    modelPath(modelPath), pPool(pPool), bufferUsageFlags(bufferUsageFlags)
	{}
	void      clear();
	template <typename Vertex>
	ModelData fetch(CmdBuffer cmdBuf, std::string path, uint32_t loadFlags)
	{
		auto vertexType = vertex_type<Vertex>();
		ModelKey key{path, std::string(typeid(vertexType).name()), loadFlags};
		auto     it = map.find(key);
		if (it == map.end())
		{
			std::string                    fullPath = modelPath + path;
			std::vector<ObjVertex>         vertexList;
			std::vector<Index>             indexList;
			ModelData modelData{};
			VkBufferUsageFlags             additionalBufferUsageFlags = 0;
			if (loadFlags & MODEL_LOAD_FLAG_CREATE_ACCELERATION_STRUCTURE)
			{
				additionalBufferUsageFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			}
			if (loadFlags & MODEL_LOAD_FLAG_COPYABLE)
			{
				additionalBufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			}
			printVka(("Loading model: " + path).c_str());
			if (loadObj(fullPath, vertexList, indexList, modelData.indexCount, modelData.mtl))
			{
				modelData.vertexLayout = vertexType.data_layout();
				Buffer stagingBuffer = createStagingBuffer();
				vertexType.load_obj(stagingBuffer, vertexList);
				modelData.vertexBuffer = createBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | bufferUsageFlags | additionalBufferUsageFlags);
				cmdUploadCopy(cmdBuf, stagingBuffer, modelData.vertexBuffer);

				modelData.indexBuffer = createBuffer(pPool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | bufferUsageFlags | additionalBufferUsageFlags);
				cmdWriteCopy(cmdBuf, modelData.indexBuffer, indexList.data(), indexList.size() * sizeof(Index));
				cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR);
				if (loadFlags & MODEL_LOAD_FLAG_CREATE_ACCELERATION_STRUCTURE)
				{
					modelData.blas = buildAccelerationStructure(cmdBuf, modelData, loadFlags);
				}

				modelData.vertexLayout = vertexType.data_layout();

				findAreaLights(modelData.lights, vertexList, indexList, modelData.indexCount, modelData.mtl);

				map.insert({key, modelData});
			}
			else
			{
				printVka("Failed to load model");
			}
			return modelData;
		}
		return it->second;
	}


};
}        // namespace vka