#pragma once
#include <vka/advanced_state/AdvancedState.h>
#include <vka/core/core_common.h>
#include "../HdrImagePdfCache.h"

namespace vka
{
namespace pbr
{

// Shader struct, respect alignment rules (https://registry.khronos.org/OpenGL/extensions/ARB/ARB_uniform_buffer_object.txt)
struct ModelDataOffset
{
	uint32_t firstVertex;
	uint32_t firstSurface;
};
static_assert(offsetof(ModelDataOffset, firstSurface) == 1 * 4, "Offset is not correct");
static_assert(sizeof(ModelDataOffset) == 8, "Size is not correct");

template <class Material>
struct material_type;

struct USceneInfo
{
	bool useAreaLight = false;
	bool useEnvMap    = false;
	bool useTextures  = false;
};

class USceneData
{
  public:
	Buffer             vertexBuffer;
	Buffer             indexBuffer;
	Buffer             modelOffsetBuffer;
	Buffer             surfaceOffsetBuffer;
	Buffer             materialBuffer;
	Buffer             areaLightBuffer;
	TLAS               tlas;

	std::vector<Image> textures;
	Image              envMap;
	Buffer             envMapPdfBuffer;

	USceneInfo info;

	void garbageCollect()
	{
		vertexBuffer->garbageCollect();
		indexBuffer->garbageCollect();
		modelOffsetBuffer->garbageCollect();
		surfaceOffsetBuffer->garbageCollect();
		materialBuffer->garbageCollect();
		areaLightBuffer->garbageCollect();
		tlas->garbageCollect();
	}

	// build tlas
	void build(CmdBuffer cmdBuf, Buffer instanceBuffer);
};

enum SceneLoadFlags
{
	SCENE_LOAD_FLAG_ALLOW_RASTERIZATION = 1 << 0,
};

class USceneBuilderBase
{
  protected:
	// Model related data
	std::unordered_map<std::string, uint32_t> textureIndexMap;
	std::vector<ModelData>                    modelList;
	std::vector<glm::mat4>                    transformList;
	std::unordered_map<std::string, uint32_t> indexMap;


	// Ohter data
	std::string envMapName;

	HdrImagePdfCache* pdfCache;

	// create material buffer
	virtual void loadMaterials(CmdBuffer cmdBuf, Buffer buffer) = 0;

  public:
	
	USceneBuilderBase(HdrImagePdfCache* pdfCache) : pdfCache(pdfCache) {}

	void loadEnvMap(const ImagePdfKey &key);

	// load model
	void addModel(CmdBuffer cmdBuf, std::string path, glm::mat4 transform = glm::mat4(1.0), uint32_t loadFlags = MODEL_LOAD_FLAG_IS_OPAQUE);

	virtual void addModelInternal(CmdBuffer cmdBuf, ModelCache *pModelCache, std::string path, uint32_t loadFlags) = 0;

	// copy together buffers, create tlas
	USceneData create(CmdBuffer cmdBuf, IResourcePool *pPool, uint32_t sceneLoadFlags);

	// create and upload instance buffer
	Buffer uploadInstanceData(CmdBuffer cmdBuf, IResourcePool *pPool);

	// set transform for model
	void setTransform(std::string path, glm::mat4 transform);

	// revert to initial state
	void reset();
};

template <class Vertex, class Material>
class USceneBuilder : public USceneBuilderBase
{

	// create material buffer
	virtual void loadMaterials(CmdBuffer cmdBuf, Buffer buffer) override
	{
		std::vector<Material> materialList;
		for (auto& model : modelList)
		{
			for (auto& mtl : model.mtl)
			{
				materialList.push_back(material_type<Material>().load_mtl(mtl, textureIndexMap));
			}
		}
		cmdWriteCopy(cmdBuf, buffer, materialList.data(), materialList.size() * sizeof(Material));
	}
	void addModelInternal(CmdBuffer cmdBuf, ModelCache *pModelCache, std::string path, uint32_t loadFlags) override
	{
		modelList.push_back(pModelCache->fetch<Vertex>(cmdBuf, path, loadFlags));
	}

  public:

	USceneBuilder(HdrImagePdfCache *pdfCache) :
	    USceneBuilderBase(pdfCache)
	{}
	// load model

};

}        // namespace pbr
}        // namespace vka
