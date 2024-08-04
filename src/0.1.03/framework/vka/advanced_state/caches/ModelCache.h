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
	MODEL_LOAD_FLAG_IS_OPAQUE = 1 << 1
};

struct ModelData
{
	Buffer                vertexBuffer = nullptr;
	Buffer                indexBuffer  = nullptr;
	BLAS                  blas         = nullptr;


	VertexDataLayout	  vertexLayout;
	std::vector<uint32_t> indexOffsets;
	uint32_t              indexCount = 0;


	bool operator==(const ModelData &other) const
	{
		return vertexBuffer == other.vertexBuffer && indexBuffer == other.indexBuffer && cmpVector(indexOffsets, other.indexOffsets) && blas == other.blas;
	};
	vka::hash_t hash() const
	{
		return vertexBuffer HASHC indexBuffer HASHC hashVector(indexOffsets) HASHC blas;
	}

	DrawSurface getSurface(uint32_t idx) const
	{
		if (idx >= indexOffsets.size() - 1)
			return {vertexBuffer, indexBuffer, indexOffsets.back(), indexCount - indexOffsets.back(), vertexLayout};
		else
			return {vertexBuffer, indexBuffer, indexOffsets[idx], indexOffsets[idx + 1] - indexOffsets[idx], vertexLayout};
	}
};

struct ModelKey
{
	std::string      path;
	void            *loadFunction;
	uint32_t         loadFlags;

	bool operator==(const ModelKey &other) const
	{
		return path == other.path && loadFunction == loadFunction && loadFlags == other.loadFlags;
	}

	vka::hash_t hash() const
	{
		return std::hash<std::string>()(path) ^ std::hash<void *>()(loadFunction) ^ loadFlags;
	}
};
}		// namespace vka
DECLARE_HASH(vka::ModelData, hash);
DECLARE_HASH(vka::ModelKey, hash);
DECLARE_HASH(vka::ObjVertex, hash);

namespace vka
{
class ModelCache
{
	std::unordered_map<ModelKey, ModelData> map;
	std::string                             modelPath;
	IResourcePool                          *pPool;
	VkBufferUsageFlags                      bufferUsageFlags;

  public:
	ModelCache(IResourcePool *pPool, std::string modelPath, VkBufferUsageFlags bufferUsageFlags) :
	    modelPath(modelPath), pPool(pPool), bufferUsageFlags(bufferUsageFlags)
	{}
	void      clear();
	ModelData fetch(CmdBuffer cmdBuf, std::string path, void (*parse)(Buffer vertexBuffer, VertexDataLayout &vertexLayout, const std::vector<ObjVertex> &vertexList), uint32_t loadFlags);
};
}        // namespace vka