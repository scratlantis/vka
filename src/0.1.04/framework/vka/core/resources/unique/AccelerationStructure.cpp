#include "AccelerationStructure.h"
#include <vka/globals.h>
#include <vka/core/core_state/device_properties.h>


namespace vka
{
void AccelerationStructureVK_R::free()
{
	LOAD_CMD_VK_DEVICE(vkDestroyAccelerationStructureKHR, gState.device.logical);
	pvkDestroyAccelerationStructureKHR(gState.device.logical, handle, nullptr);
}

const VkAccelerationStructureGeometryKHR *BottomLevelAS_R::getGeometryPtr() const
{
	return geometry.data();
};

uint32_t BottomLevelAS_R::getGeometryCount() const
{
	return geometry.size();
};

std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> BottomLevelAS_R::getBuildRangePtrs() const
{
	std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> ptrs;
	for (auto &range : buildRange)
	{
		ptrs.push_back(&range);
	}
	return ptrs;
}

void BottomLevelAS_R::setGeometry(std::vector<VkAccelerationStructureGeometryKHR> &geom)
{
	geometry = geom;
};

void BottomLevelAS_R::setBuildRange(std::vector<VkAccelerationStructureBuildRangeInfoKHR> &range)
{
	buildRange = range;
};

const BottomLevelAS_R BottomLevelAS_R::getShallowCopy() const
{
	return *this;
}

VkAccelerationStructureBuildGeometryInfoKHR BottomLevelAS_R::getBuildInfoInternal() const
{
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
	buildInfo.flags                    = buildFlags;
	buildInfo.geometryCount            = geometry.size();
	buildInfo.pGeometries              = geometry.data();
	buildInfo.type                     = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	return buildInfo;
}


VkDeviceSize BottomLevelAS_R::getBuildSize() const
{
	LOAD_CMD_VK_DEVICE(vkGetAccelerationStructureBuildSizesKHR, gState.device.logical);
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = getBuildInfoInternal();
	std::vector<uint32_t> maxPrimCount(buildRange.size());
	for (uint32_t i = 0; i < buildRange.size(); i++)
	{
		maxPrimCount[i] = buildRange[i].primitiveCount;
	}
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
	pvkGetAccelerationStructureBuildSizesKHR(gState.device.logical, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, maxPrimCount.data(), &sizeInfo);
	return sizeInfo.accelerationStructureSize;
}

VkDeviceSize BottomLevelAS_R::getScratchSize() const
{
	LOAD_CMD_VK_DEVICE(vkGetAccelerationStructureBuildSizesKHR, gState.device.logical);
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = getBuildInfoInternal();
	std::vector<uint32_t>                       maxPrimCount(buildRange.size());
	for (uint32_t i = 0; i < buildRange.size(); i++)
	{
		maxPrimCount[i] = buildRange[i].primitiveCount;
	}
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
	pvkGetAccelerationStructureBuildSizesKHR(gState.device.logical, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, maxPrimCount.data(), &sizeInfo);
	return sizeInfo.buildScratchSize;
}

VkAccelerationStructureTypeKHR BottomLevelAS_R::getType() const
{
	return VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
};



TopLevelAS_R::TopLevelAS_R(IResourcePool *pPool) :
    AccelerationStructure_R(pPool)
{
	VkAccelerationStructureGeometryInstancesDataKHR instancesVk{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR};
	buildGeom                    = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
	buildGeom.geometryType       = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	buildGeom.geometry.instances = instancesVk;
}

void TopLevelAS_R::setInstanceCount(uint32_t count)
{
	instanceCount = count;
}

void TopLevelAS_R::setInstanceData(Buffer_R *instanceBuffer)
{
	buildGeom.geometry.instances.data.deviceAddress = instanceBuffer->getDeviceAddress();
}

std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> TopLevelAS_R::getBuildRangePtrs() const
{
	// Build Offsets info
	std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> ptrs;
	ptrs.push_back(&buildRange);
	return ptrs;
}

const TopLevelAS_R TopLevelAS_R::getShallowCopy() const
{
	return *this;
}
VkAccelerationStructureBuildGeometryInfoKHR TopLevelAS_R::getBuildInfoInternal() const
{
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
	buildInfo.flags         = buildFlags;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries   = &buildGeom;
	buildInfo.type          = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	return buildInfo;
}

VkDeviceSize TopLevelAS_R::getBuildSize() const
{
	LOAD_CMD_VK_DEVICE(vkGetAccelerationStructureBuildSizesKHR, gState.device.logical);
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = getBuildInfoInternal();
	VkAccelerationStructureBuildSizesInfoKHR    sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
	pvkGetAccelerationStructureBuildSizesKHR(gState.device.logical, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &buildRange.primitiveCount, &sizeInfo);
	return sizeInfo.accelerationStructureSize;
}

VkDeviceSize TopLevelAS_R::getScratchSize() const
{
	LOAD_CMD_VK_DEVICE(vkGetAccelerationStructureBuildSizesKHR, gState.device.logical);
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = getBuildInfoInternal();
	VkAccelerationStructureBuildSizesInfoKHR    sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
	pvkGetAccelerationStructureBuildSizesKHR(gState.device.logical, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &buildRange.primitiveCount, &sizeInfo);
	return sizeInfo.buildScratchSize;
}

VkAccelerationStructureTypeKHR TopLevelAS_R::getType() const
{
	return VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
}

void TopLevelAS_R::createHandles()
{
	buildRange.primitiveCount = instanceCount;
	AccelerationStructure_R::createHandles();
}

void TopLevelAS_R::recreate()
{
	if (instanceCount == 0)
	{
		throw std::runtime_error("TopLevelAS_R::recreate() called with instanceCount == 0");
	}
	else if (asRes != nullptr && instanceCount == buildRange.primitiveCount)
	{
		return;
	}
	AccelerationStructure_R::recreate();
}

bool AccelerationStructure_R::isBuilt() const
{
	return built;
}
VkAccelerationStructureBuildGeometryInfoKHR AccelerationStructure_R::getBuildInfo(VkAccelerationStructureKHR src, Buffer_R *scratchBuffer) const
{
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = getBuildInfoInternal();
	buildInfo.srcAccelerationStructure                    = src;
	buildInfo.mode                                        = (src == VK_NULL_HANDLE) ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	buildInfo.dstAccelerationStructure                    = handle;
	buildInfo.scratchData.deviceAddress                   = alignUp(scratchBuffer->getDeviceAddress(), getAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);
	return buildInfo;
}
void AccelerationStructure_R::configureScratchBuffer(Buffer_R *scratchBuffer) const
{
	VkPhysicalDeviceAccelerationStructurePropertiesKHR asProp = getAccelerationStructureProperties();
	scratchBuffer->changeSize(std::max(scratchBuffer->getSize(), alignUp(getScratchSize() + asProp.minAccelerationStructureScratchOffsetAlignment, asProp.minAccelerationStructureScratchOffsetAlignment)));
	scratchBuffer->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	scratchBuffer->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
}
void AccelerationStructure_R::setBuilt(bool built)
{
	this->built = built;
}
void AccelerationStructure_R::setBuildFlags(VkBuildAccelerationStructureFlagsKHR flags)
{
	buildFlags = flags;
}
void AccelerationStructure_R::track(IResourcePool *pPool)
{
	if (asRes)
	{
		asRes->track(pPool);
	}
	if (bufRes)
	{
		bufRes->track(pPool);
	}
	Resource::track(pPool);
}
hash_t AccelerationStructure_R::hash() const
{
	return asRes->hash() << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}
void AccelerationStructure_R::createHandles()
{
	LOAD_CMD_VK_DEVICE(vkCreateAccelerationStructureKHR, gState.device.logical);
	VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
	createInfo.type = getType();
	createInfo.size = getBuildSize();
	bufRes->changeSize(createInfo.size);
	bufRes->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	bufRes->recreate();
	createInfo.buffer = bufRes->getHandle();
	VK_CHECK(pvkCreateAccelerationStructureKHR(gState.device.logical, &createInfo, nullptr, &handle));
	asRes = new AccelerationStructureVK_R(handle);
	asRes->track(getPool());
}
void AccelerationStructure_R::detachChildResources()
{
	if (asRes)
	{
		asRes->track(gState.frame->stack);
		asRes = nullptr;
	}
	if (bufRes)
	{
		bufRes->track(gState.frame->stack);
		bufRes = nullptr;
	}
}
void AccelerationStructure_R::recreate()
{
	detachChildResources();
	createHandles();
	built = false;
}

VkDeviceAddress AccelerationStructure_R::getDeviceAddress() const
{
	LOAD_CMD_VK_DEVICE(vkGetAccelerationStructureDeviceAddressKHR, gState.device.logical);
	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR};
	addressInfo.accelerationStructure = handle;
	return pvkGetAccelerationStructureDeviceAddressKHR(gState.device.logical, &addressInfo);
}

}        // namespace vka