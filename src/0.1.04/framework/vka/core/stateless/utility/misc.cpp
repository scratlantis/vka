#include "misc.h"
#include <glm/ext/matrix_transform.hpp>
#include "io.h"
namespace vka
{

glm::mat4 getMatrix(glm::vec3 pos, glm::vec3 rotDeg, float scale)
{
	glm::mat4 objToWorld = glm::mat4(1.0f);
	objToWorld           = glm::translate(objToWorld, pos);
	objToWorld           = glm::rotate(objToWorld, glm::radians(rotDeg.x), glm::vec3(1.0f, 0.0f, 0.0f));
	objToWorld           = glm::rotate(objToWorld, glm::radians(rotDeg.y), glm::vec3(0.0f, 1.0f, 0.0f));
	objToWorld           = glm::rotate(objToWorld, glm::radians(rotDeg.z), glm::vec3(0.0f, 0.0f, 1.0f));
	objToWorld           = glm::scale(objToWorld, glm::vec3(scale));
	return objToWorld;
};



// from https://github.com/vcoda/magma
void *copyBinaryData(const void *src, std::size_t size) noexcept
{
	void *dst = new char[size];
	if (dst)
		memcpy(dst, src, size);
	return dst;
}

std::vector<uint8_t> getByteVector(void *data, size_t size)
{
	std::vector<uint8_t> out(size);
	memcpy(out.data(), data, size);
	return out;
}

glm::uvec3 getWorkGroupCount(const glm::uvec3 &workGroupSize, const glm::uvec3 &resolution)
{
	return {alignUp(resolution.x, workGroupSize.x) / workGroupSize.x,
	        alignUp(resolution.y, workGroupSize.y) / workGroupSize.y,
	        alignUp(resolution.z, workGroupSize.z) / workGroupSize.z};
}

std::vector<uint32_t> glm3VectorSizes()
{
	return {sizeof(uint32_t), sizeof(uint32_t), sizeof(uint32_t)};
}

std::vector<uint8_t> getByteVector(const glm::uvec3 &in)
{
	uint32_t             v[] = {in.x, in.y, in.z};
	std::vector<uint8_t> out(3 * sizeof(uint32_t));
	memcpy(out.data(), v, 3 * sizeof(uint32_t));
	return out;
}

uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	DEBUG_BREAK
	return UINT32_MAX;        // ToDo throw error message
}


void submit(
    std::vector<VkCommandBuffer> cmdBufs, VkQueue queue, const SubmitSynchronizationInfo syncInfo)
{
	VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit.waitSemaphoreCount   = syncInfo.waitSemaphores.size();
	submit.pWaitSemaphores      = syncInfo.waitSemaphores.data();
	submit.pWaitDstStageMask    = syncInfo.waitDstStageMask.data();
	submit.signalSemaphoreCount = syncInfo.signalSemaphores.size();
	submit.pSignalSemaphores    = syncInfo.signalSemaphores.data();
	submit.pCommandBuffers      = cmdBufs.data();
	submit.commandBufferCount   = cmdBufs.size();
	VK_CHECK(vkQueueSubmit(queue, 1, &submit, syncInfo.signalFence));
}

void submit(VkCommandBuffer cmdBuf, VkQueue queue, const SubmitSynchronizationInfo syncInfo)
{
	VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit.waitSemaphoreCount   = syncInfo.waitSemaphores.size();
	submit.pWaitSemaphores      = syncInfo.waitSemaphores.data();
	submit.pWaitDstStageMask    = syncInfo.waitDstStageMask.data();
	submit.signalSemaphoreCount = syncInfo.signalSemaphores.size();
	submit.pSignalSemaphores    = syncInfo.signalSemaphores.data();
	submit.pCommandBuffers      = &cmdBuf;
	submit.commandBufferCount   = 1;
	VK_CHECK(vkQueueSubmit(queue, 1, &submit, syncInfo.signalFence));
}

VkSpecializationInfo writeSpecializationInfo(
    const uint32_t                         mapEntriesCount,
    std::vector<VkSpecializationMapEntry> &mapEntries, const std::vector<uint32_t> &entrySizes, uint32_t &mapEntryOffset,
    const void *data, uint32_t &dataOffset)
{
	VkSpecializationInfo specInfo{};
	specInfo.pData         = (char *) data + dataOffset;
	specInfo.dataSize      = 0;
	specInfo.pMapEntries   = &mapEntries[mapEntryOffset];
	specInfo.mapEntryCount = mapEntriesCount;
	for (uint32_t i = 0; i < mapEntriesCount; i++)
	{
		VkSpecializationMapEntry specEntry{};
		specEntry.constantID = i;
		specEntry.offset     = specInfo.dataSize;
		specEntry.size       = entrySizes[i + mapEntryOffset];
		specInfo.dataSize += specEntry.size;
		mapEntries[i + mapEntryOffset] = specEntry;
	}
	mapEntryOffset += mapEntriesCount;
	dataOffset += specInfo.dataSize;
	return specInfo;
}

void writeSpecializationInfo(
    const std::vector<uint32_t>           &entrySizes,
    const void                            *data,
    std::vector<VkSpecializationMapEntry> &mapEntries,
    VkSpecializationInfo                  &specInfo)
{
	mapEntries.resize(entrySizes.size());
	uint32_t mapEntryOffset = 0;
	uint32_t dataOffset     = 0;
	specInfo                = writeSpecializationInfo(entrySizes.size(), mapEntries, entrySizes, mapEntryOffset, data, dataOffset);
}

void writeSpecializationInfos(
    const std::vector<uint32_t>           &entryCounts,
    const std::vector<uint32_t>           &entrySizes,
    const void                            *data,
    std::vector<VkSpecializationMapEntry> &mapEntries,
    std::vector<VkSpecializationInfo>     &specInfos)
{
	uint32_t totalMapEntryCount = 0;
	for (size_t i = 0; i < entryCounts.size(); i++)
	{
		totalMapEntryCount += entryCounts[i];
	}
	mapEntries.resize(totalMapEntryCount);
	specInfos.resize(entryCounts.size());

	uint32_t mapEntryOffset = 0;
	uint32_t dataOffset     = 0;
	for (size_t i = 0; i < entryCounts.size(); i++)
	{
		specInfos[i] = writeSpecializationInfo(entryCounts[i], mapEntries, entrySizes, mapEntryOffset, data, dataOffset);
	}
}


VkImageAspectFlags getAspectFlags(VkFormat format)
{
	VkImageAspectFlags aspectFlags = 0;
	if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D16_UNORM_S8_UINT ||
	    format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	return aspectFlags;
}

VkAccessFlags getAccessFlags(VkImageLayout layout)
{
	switch (layout)
	{
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			return VK_ACCESS_HOST_WRITE_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_ACCESS_TRANSFER_WRITE_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_ACCESS_SHADER_READ_BIT;
		case VK_IMAGE_LAYOUT_GENERAL:
			return VK_ACCESS_SHADER_WRITE_BIT;
		default:
			return VkAccessFlags();
	}
}

VkPipelineStageFlags getStageFlags(VkImageLayout oldImageLayout)
{
	switch (oldImageLayout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			return VK_PIPELINE_STAGE_HOST_BIT;
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		case VK_IMAGE_LAYOUT_GENERAL:
			return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		default:
			return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
}

VkVertexInputBindingDescription getVertexBindingDescription(uint32_t size, uint32_t binding)
{
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding   = binding;
	vertexInputBindingDescription.stride    = size;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return vertexInputBindingDescription;
}

VkVertexInputBindingDescription getInstanceBindingDescription(uint32_t size, uint32_t binding)
{
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding   = binding;
	vertexInputBindingDescription.stride    = size;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	return vertexInputBindingDescription;
}

VkTransformMatrixKHR glmToVk(glm::mat4 mat)
{
	VkTransformMatrixKHR transform;
	transform.matrix[0][0] = mat[0][0];
	transform.matrix[0][1] = mat[1][0];
	transform.matrix[0][2] = mat[2][0];
	transform.matrix[0][3] = mat[3][0];

	transform.matrix[1][0] = mat[0][1];
	transform.matrix[1][1] = mat[1][1];
	transform.matrix[1][2] = mat[2][1];
	transform.matrix[1][3] = mat[3][1];

	transform.matrix[2][0] = mat[0][2];
	transform.matrix[2][1] = mat[1][2];
	transform.matrix[2][2] = mat[2][2];
	transform.matrix[2][3] = mat[3][2];

	return transform;
}

std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, std::vector<VkFormat> formats, std::vector<uint32_t> offsets)
{
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
	for (size_t i = 0; i < formats.size(); i++)
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.location = i;
		attributeDescription.format   = formats[i];
		attributeDescription.offset   = offsets[i];
		attributeDescription.binding  = binding;
		vertexInputAttributeDescriptions.push_back(attributeDescription);
	}
	return vertexInputAttributeDescriptions;
}

// https://www.khronos.org/opengl/wiki/Image_Load_Store#Format_qualifiers
std::string getGLSLFormat(VkFormat format)
{
	switch (format)
	{
		case VK_FORMAT_R8G8B8A8_UNORM:
			return "rgba8";
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return "rgba32f";
		case VK_FORMAT_R32G32B32A32_UINT:
			return "rgba32ui";
		case VK_FORMAT_R32_SFLOAT:
			return "r32f";
			break;

		default:
			printVka("getGLSLFormat: Unsupported format!");
			DEBUG_BREAK;
			break;
	}
}


}