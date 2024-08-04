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

  private:
	VkAccelerationStructureKHR handle;
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

	void createHandles();
	void detachChildResources();
	void recreate();
};

class BottomLevelAS_R : public AccelerationStructure_R
{
  protected:
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRange;
	std::vector<VkAccelerationStructureGeometryKHR>       geometry;

	BottomLevelAS_R(const BottomLevelAS_R &rhs) = default;

	VkAccelerationStructureBuildGeometryInfoKHR getBuildInfoInternal() const;
	VkDeviceSize                                getBuildSize() const;
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

class TopLevelAs_R : public AccelerationStructure_R
{
  protected:
	VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
	VkAccelerationStructureGeometryKHR       buildGeom  = {};
	TopLevelAs_R(const TopLevelAs_R &rhs)               = default;

  private:
	VkAccelerationStructureBuildGeometryInfoKHR getBuildInfoInternal() const;
	VkDeviceSize                                getBuildSize() const;
	virtual VkAccelerationStructureTypeKHR      getType() const;

  public:
	TopLevelAs_R();
	TopLevelAs_R(IResourcePool *pPool) :
	    AccelerationStructure_R(pPool){};
	void                                                                  setInstanceCount(uint32_t count);
	void                                                                  setInstanceData(Buffer_R *instanceBuffer);
	virtual std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> getBuildRangePtrs() const;
	const TopLevelAs_R                                                    getShallowCopy() const;
};
}        // namespace vka
