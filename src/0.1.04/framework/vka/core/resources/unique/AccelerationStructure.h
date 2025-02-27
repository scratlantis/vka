#pragma once
#include "../Resource.h"
#include "Buffer.h"

namespace vka
{

class AccelerationStructureVK_R : public Resource_T<VkAccelerationStructureKHR>
{
  public:
	AccelerationStructureVK_R(VkAccelerationStructureKHR handle) :
	    Resource_T<VkAccelerationStructureKHR>(handle){};

  protected:
	void free();
};

class AccelerationStructure_R : public Resource_T<VkAccelerationStructureKHR>
{
  protected:
	Resource_T<VkAccelerationStructureKHR> *asRes      = nullptr;
	Buffer_R                               *bufRes     = nullptr;
	bool                                    built      = false;
	VkBuildAccelerationStructureFlagsKHR    buildFlags = 0;


	virtual VkAccelerationStructureBuildGeometryInfoKHR getBuildInfoInternal() const = 0;
	virtual VkDeviceSize                                getBuildSize() const         = 0;
	virtual VkDeviceSize                                getScratchSize() const       = 0;
	virtual VkAccelerationStructureTypeKHR              getType() const              = 0;

  public:
	AccelerationStructure_R() = default;
	AccelerationStructure_R(IResourcePool *pPool) :
	    Resource_T<VkAccelerationStructureKHR>(VK_NULL_HANDLE)
	{
		bufRes = new Buffer_R(pPool, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		Resource::track(pPool);
	};

	bool                                                                  isBuilt() const;
	virtual std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> getBuildRangePtrs() const = 0;
	void                                                                  configureScratchBuffer(Buffer_R *scratchBuffer) const;
	VkAccelerationStructureBuildGeometryInfoKHR                           getBuildInfo(VkAccelerationStructureKHR src, Buffer_R *scratchBuffer) const;


	void setBuilt(bool built);
	void setBuildFlags(VkBuildAccelerationStructureFlagsKHR flags);

	void   free(){};
	void   track(IResourcePool *pPool) override;
	hash_t hash() const override;

	virtual void createHandles();
	void detachChildResources();
	virtual void     recreate();
	VkDeviceAddress getDeviceAddress() const;
};

class BottomLevelAS_R : public AccelerationStructure_R
{
  protected:
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange;
	std::vector<VkAccelerationStructureGeometryKHR>       geometry;

	BottomLevelAS_R(const BottomLevelAS_R &rhs) = default;

	VkAccelerationStructureBuildGeometryInfoKHR getBuildInfoInternal() const;
	VkDeviceSize                                getBuildSize() const;
	VkDeviceSize                                getScratchSize() const;
	virtual VkAccelerationStructureTypeKHR      getType() const;

  public:
	BottomLevelAS_R() = default;
	BottomLevelAS_R(IResourcePool *pPool) :
	    AccelerationStructure_R(pPool){};

	const VkAccelerationStructureGeometryKHR                     *getGeometryPtr() const;
	uint32_t                                                      getGeometryCount() const;
	std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> getBuildRangePtrs() const;
	void                                                          setGeometry(std::vector<VkAccelerationStructureGeometryKHR> &geom);
	void                                                          setBuildRange(std::vector<VkAccelerationStructureBuildRangeInfoKHR> &range);
	const BottomLevelAS_R                                         getShallowCopy() const;
};

class TopLevelAS_R : public AccelerationStructure_R
{
  protected:
	VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
	VkAccelerationStructureGeometryKHR       buildGeom  = {};
	uint32_t                                 instanceCount;
	TopLevelAS_R(const TopLevelAS_R &rhs)               = default;

  private:
	VkAccelerationStructureBuildGeometryInfoKHR getBuildInfoInternal() const;
	VkDeviceSize                                getBuildSize() const;
	VkDeviceSize                                getScratchSize() const;

	virtual VkAccelerationStructureTypeKHR      getType() const;

	virtual ResourceType type() const override
	{
		return RESOURCE_TYPE_TLAS;
	}

  public:
	TopLevelAS_R() = default;
	TopLevelAS_R(IResourcePool *pPool);
	void                                                                  setInstanceCount(uint32_t count);
	void                                                                  setInstanceData(Buffer_R *instanceBuffer);
	virtual std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> getBuildRangePtrs() const;
	const TopLevelAS_R                                                    getShallowCopy() const;
	void                                                                  createHandles() override;
	void                                                                  recreate() override;
};
}        // namespace vka
