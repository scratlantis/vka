#pragma once
#include <vka/advanced_state/AdvancedState.h>
#include <vka/core/core_common.h>

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

template <class Instance>
struct instance_type;

struct USceneInfo
{
	bool useAreaLight = false;
	bool useEnvMap    = false;
	bool useTextures  = false;
};


struct USceneInstanceData
{
	Buffer instanceBuffer;
	Buffer tlasInstanceBuffer;
	Buffer modelIndexBuffer;
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


	Buffer instanceBuffer;
	Buffer instanceOffsetBuffer;
	TLAS               tlas;


	std::vector<Image> textures;
	Image              envMap;

	glm::uvec2 envMapSubdivisions;
	Buffer     envMapPdfBuffer;
	uint32_t   areaLightCount;

	USceneInfo info;
	bool isInitialized = false;

	void garbageCollect()
	{
		if (!isInitialized)
		{
			return;
		}
		vertexBuffer->garbageCollect();
		indexBuffer->garbageCollect();
		modelOffsetBuffer->garbageCollect();
		surfaceOffsetBuffer->garbageCollect();
		materialBuffer->garbageCollect();
		areaLightBuffer->garbageCollect();
		tlas->garbageCollect();
	}

	// build tlas
	void build(CmdBuffer cmdBuf, USceneInstanceData instanceBuffer);
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
	std::unordered_map<std::string, uint32_t> indexMap;


	// Ohter data
	std::string envMapName;
	glm::uvec2  envMapSubdivisions;


	// create material buffer
	virtual void loadMaterials(CmdBuffer cmdBuf, Buffer buffer) = 0;

  public:
	
	USceneBuilderBase(){}

	void loadEnvMap(std::string name, glm::uvec2 subdivisions);

	// load model
	//void addModel(CmdBuffer cmdBuf, std::string path, glm::mat4 transform = glm::mat4(1.0), uint32_t loadFlags = MODEL_LOAD_FLAG_IS_OPAQUE);

	void addModel(CmdBuffer cmdBuf, std::string path, void *instanceData, uint32_t instanceCount, uint32_t loadFlags = MODEL_LOAD_FLAG_IS_OPAQUE);
	void addModel(CmdBuffer cmdBuf, std::string path, Buffer instanceBuffer, uint32_t instanceCount, uint32_t loadFlags = MODEL_LOAD_FLAG_IS_OPAQUE);

	virtual void addModelInternal(CmdBuffer cmdBuf, ModelCache *pModelCache, std::string path, void* instanceData, uint32_t instanceCount, uint32_t loadFlags) = 0;
	virtual void addModelInternal(CmdBuffer cmdBuf, ModelCache *pModelCache, std::string path, Buffer instanceBuffer, uint32_t instanceCount, uint32_t loadFlags) = 0;
	virtual USceneInstanceData uploadInstanceData(CmdBuffer cmdBuf, IResourcePool *pPool)                                                                                       = 0;
	virtual void               reset()                                                                                                                                          = 0;
	virtual uint32_t           getInstanceOffset(uint32_t modelIndex)                                                                                                           = 0;
	virtual uint32_t           getInstanceCount(uint32_t modelIndex)                                                                                                            = 0;
	virtual uint32_t           getTotalInstanceCount()                                                                                                                          = 0;
	// copy together buffers, create tlas
	USceneData create(CmdBuffer cmdBuf, IResourcePool *pPool, uint32_t sceneLoadFlags = 0);
};

template <class Vertex, class Material, class Instance>
class USceneBuilder : public USceneBuilderBase
{
	struct InstanceData
	{
		uint32_t modelIndex;
		uint32_t instanceCount;
		std::vector<Instance> hostData;
		Buffer deviceData;
	};
	std::vector<InstanceData> instanceList;
	std::vector<uint32_t> modelIndexList;
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

	virtual void addModelInternal(CmdBuffer cmdBuf, ModelCache* pModelCache, std::string path, void* instanceData, uint32_t instanceCount, uint32_t loadFlags) override
	{
		ModelData model = pModelCache->fetch<Vertex>(cmdBuf, path, loadFlags);
		InstanceData inst{};
		inst.modelIndex = modelList.size();
		inst.instanceCount = instanceCount;
		inst.hostData = std::vector<Instance>((Instance*)instanceData, (Instance*)instanceData + instanceCount);//civ
		uint32_t modelIndex = modelList.size();
		for (size_t i = 0; i < instanceCount; i++)
		{
			modelIndexList.push_back(modelIndex);
		}
		modelList.push_back(model);
		instanceList.push_back(inst);
	}
	virtual void addModelInternal(CmdBuffer cmdBuf, ModelCache* pModelCache, std::string path, Buffer instanceBuffer, uint32_t instanceCount, uint32_t loadFlags) override
	{
		ModelData    model = pModelCache->fetch<Vertex>(cmdBuf, path, loadFlags);
		InstanceData inst{};
		inst.modelIndex    = modelList.size();
		inst.instanceCount = instanceCount;
		inst.deviceData    = instanceBuffer;
		uint32_t modelIndex = modelList.size();
		for (size_t i = 0; i < instanceCount; i++)
		{
			modelIndexList.push_back(modelIndex);
		}
		modelList.push_back(model);
		instanceList.push_back(inst);
	}


  public:
	virtual USceneInstanceData uploadInstanceData(CmdBuffer cmdBuf, IResourcePool *pPool)
	{
		USceneInstanceData instanceData{};

		instanceData.modelIndexBuffer   = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, modelIndexList.size() * sizeof(uint32_t));
		cmdWriteCopy(cmdBuf, instanceData.modelIndexBuffer, modelIndexList.data(), modelIndexList.size() * sizeof(uint32_t));

		instanceData.tlasInstanceBuffer = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY);
		std::vector<VkAccelerationStructureInstanceKHR> tlasInstances;
		for (auto& inst : instanceList)
		{
			for (uint32_t i = 0; i < inst.instanceCount; i++)
			{
				// transform and mask written on gpu
				VkAccelerationStructureInstanceKHR tlasInstance{};
				tlasInstance.instanceCustomIndex                    = tlasInstances.size();
				tlasInstance.instanceShaderBindingTableRecordOffset = 0;
				tlasInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
				tlasInstance.accelerationStructureReference = modelList[inst.modelIndex].blas->getDeviceAddress();

				tlasInstances.push_back(tlasInstance);
			}

			if (!inst.hostData.empty())
			{
				VKA_ASSERT(inst.hostData.size() == inst.instanceCount);
				inst.deviceData = createStagingBuffer();
				write(inst.deviceData, inst.hostData.data(), inst.hostData.size() * sizeof(Instance));
			}
		}
		uint32_t totalInstanceCount = tlasInstances.size();
		cmdWriteCopy(cmdBuf, instanceData.tlasInstanceBuffer, tlasInstances.data(), totalInstanceCount * sizeof(VkAccelerationStructureInstanceKHR));

		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);

		instanceData.instanceBuffer   = createBuffer(pPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, totalInstanceCount * sizeof(Instance));
		uint32_t currentInstanceCount = 0;
		for (auto &inst : instanceList)
		{
			cmdCopyBufferRegion(cmdBuf, inst.deviceData, instanceData.instanceBuffer, 0, currentInstanceCount * sizeof(Instance), inst.instanceCount * sizeof(Instance));
			currentInstanceCount += inst.instanceCount;
		}

		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

		instance_type<Instance>().get_cmd_write_tlas_instance(instanceData.instanceBuffer, instanceData.tlasInstanceBuffer, totalInstanceCount).exec(cmdBuf);
		return instanceData;
	}
	virtual void reset() override
	{
		textureIndexMap.clear();
		modelList.clear();
		instanceList.clear();
		indexMap.clear();
		envMapName = "";
	}
	virtual uint32_t getInstanceOffset(uint32_t modelIndex)
	{
		uint32_t offset = 0;
		for (uint32_t i = 0; i < modelIndex; i++)
		{
			offset += instanceList[i].instanceCount;
		}
		return offset;
	}
	virtual uint32_t getInstanceCount(uint32_t modelIndex)
	{
		return instanceList[modelIndex].instanceCount;
	}
	virtual uint32_t getTotalInstanceCount() override
	{
		uint32_t count = 0;
		for (auto& inst : instanceList)
		{
			count += inst.instanceCount;
		}
		return count;
	}


	USceneBuilder() :
	    USceneBuilderBase()
	{}
	// load model

};

}        // namespace pbr
}        // namespace vka
