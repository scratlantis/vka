#include "acceleration_structure_utility.h"
namespace vka
{
BLAS createBottomLevelAS(IResourcePool *pPool, std::vector<VkAccelerationStructureGeometryKHR> &geom, std::vector<VkAccelerationStructureBuildRangeInfoKHR> &range)
{
	BLAS blas = new BottomLevelAS_R(pPool);
	blas->setGeometry(geom);
	blas->setBuildRange(range);
	blas->createHandles();
	return blas;
}

TLAS createTopLevelAS(IResourcePool* pPool, uint32_t instanceCount)
{
	TLAS tlas = new TopLevelAS_R(pPool);
	tlas->setInstanceCount(instanceCount);
	tlas->createHandles();
	return tlas;
}
}