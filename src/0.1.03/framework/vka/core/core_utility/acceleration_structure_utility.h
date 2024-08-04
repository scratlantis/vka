#pragma once
#include "types.h"
#include <vka/core/resources/IResourcePool.h>
namespace vka
{
BLAS createBottomLevelAS(IResourcePool *pPool, std::vector<VkAccelerationStructureGeometryKHR> &geom, std::vector<VkAccelerationStructureBuildRangeInfoKHR> &range);
}