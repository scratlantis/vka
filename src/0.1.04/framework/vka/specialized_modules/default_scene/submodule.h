#pragma once
#include "interface_structs.h"
#include <vka/advanced_utility/complex_commands.h>
#include <vka/pbr/universal/UScene.h>

namespace vka
{
namespace default_scene
{
struct CameraCI
{
	vec3       pos;
	vec3       frontDir;
	vec3       upDir;
	uint       frameIdx;
	VkExtent2D extent;
	float      yFovDeg;
	float      zNear;
	float      zFar;
};
void bindCamera(ComputeCmd &cmd, Buffer camBuf, Buffer camInstBuf);
void cmdUpdateCamera(CmdBuffer cmdBuf, Buffer camBuf, Buffer camInstBuf, CameraCI ci);
void bindScene(ComputeCmd &cmd, const pbr::USceneData *pScene);
void   bindMockScene(ComputeCmd &cmd);
Buffer cmdGetScalarFieldUniform(CmdBuffer cmdBuf, IResourcePool *pPool, float densityScale, float minDensity, float maxDensity);
void   bindScalarField(ComputeCmd &cmd, Image scalarField, float rayMarchStepSize, Buffer ubo);

void bindBoxIntersector(ComputeCmd &cmd, TLAS tlas);

template <typename Instance>
void cmdBuildBoxIntersector(CmdBuffer cmdBuf, BLAS srcBlas, Buffer srcInstanceBuf, uint32_t instanceCount, TLAS dstTlas)
{
	dstTlas->setInstanceCount(instanceCount);
	dstTlas->recreate();
	Buffer tlasInstanceBuf = createStagingBuffer();
	tlasInstanceBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	tlasInstanceBuf->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	std::vector<VkAccelerationStructureInstanceKHR> tlasInstances(instanceCount);
	for (size_t i = 0; i < instanceCount; i++)
	{
		tlasInstances[i]                                        = {};
		tlasInstances[i].instanceCustomIndex                    = i;
		tlasInstances[i].accelerationStructureReference         = srcBlas->getDeviceAddress();
	}
	cmdWriteCopy(cmdBuf, tlasInstanceBuf, tlasInstances.data(), tlasInstances.size() * sizeof(VkAccelerationStructureInstanceKHR));
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
	instance_type<Instance>().get_cmd_write_tlas_instance(srcInstanceBuf, tlasInstanceBuf, instanceCount).exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR);
	cmdBuildAccelerationStructure(cmdBuf, dstTlas, tlasInstanceBuf, createStagingBuffer());
}


}        // namespace default_scene
}        // namespace vka