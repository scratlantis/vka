#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "macros.h"
namespace vka
{

// General helper functions/types

template <typename T>
struct Rect2D
{
	T    x;
	T    y;
	T    width;
	T    height;
	bool operator==(const Rect2D &other) const
	{
		return x == other.x && y == other.y && width == other.width && height == other.height;
	}
};

template <class integral>
constexpr integral alignUp(integral x, size_t a) noexcept
{
	return integral((x + (integral(a) - 1)) & ~integral(a - 1));
}

// from https://github.com/vcoda/magma
inline void *copyBinaryData(const void *src, std::size_t size) noexcept
{
	void *dst = new char[size];
	if (dst)
		memcpy(dst, src, size);
	return dst;
}

template <class T>
inline void *copyBinaryData(const T &src) noexcept
{
	void *dst = new char[sizeof(T)];
	if (dst)
		memcpy(dst, &src, sizeof(T));
	return dst;
}

// T must have zero padding
template <class T>
inline bool selectByPreference(const std::vector<T> &options, const std::vector<T> &preferences, T &selection)
{
	for (size_t i = 0; i < options.size(); i++)
	{
		for (size_t j = 0; j < preferences.size(); j++)
		{
			if (std::memcmp(&options[i], &preferences[j], sizeof(T)) == 0)
			{
				selection = options[i];
				return true;
			}
		}
	}
	return false;
}

std::vector<uint8_t> inline getByteVector(void *data, size_t size)
{
	std::vector<uint8_t> out(size);
	memcpy(out.data(), data, size);
	return out;
}

template <class T>
inline uint32_t dataSize(std::vector<T> v)
{
	return static_cast<uint32_t>(v.size() * sizeof(T));
}




// Glm helper functions
glm::uvec3 inline getWorkGroupCount(const glm::uvec3 &workGroupSize, const glm::uvec3 &resolution)
{
	return {alignUp(resolution.x, workGroupSize.x) / workGroupSize.x,
	        alignUp(resolution.y, workGroupSize.y) / workGroupSize.y,
	        alignUp(resolution.z, workGroupSize.z) / workGroupSize.z};
}

std::vector<uint32_t> inline glm3VectorSizes()
{
	return {sizeof(uint32_t), sizeof(uint32_t), sizeof(uint32_t)};
}

std::vector<uint8_t> inline getByteVector(const glm::uvec3 &in)
{
	uint32_t             v[] = {in.x, in.y, in.z};
	std::vector<uint8_t> out(3 * sizeof(uint32_t));
	memcpy(out.data(), v, 3 * sizeof(uint32_t));
	return out;
}



// Vulkan helper functions

struct SubmitSynchronizationInfo
{
	std::vector<VkSemaphore>          waitSemaphores{};
	std::vector<VkSemaphore>          signalSemaphores{};
	std::vector<VkPipelineStageFlags> waitDstStageMask{};
	VkFence                           signalFence = VK_NULL_HANDLE;
};

constexpr void clamp(VkExtent2D &target, const VkExtent2D &minExtent, const VkExtent2D &maxExtent)
{
	target.width  = std::max(minExtent.width, std::min(maxExtent.width, target.width));
	target.height = std::max(minExtent.height, std::min(maxExtent.height, target.height));
}

inline uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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



inline void submit(
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

inline void submit(VkCommandBuffer cmdBuf, VkQueue queue, const SubmitSynchronizationInfo syncInfo)
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

constexpr VkExtent3D getExtent3D(const VkExtent2D extent)
{
	return {extent.width, extent.height, 1};
}

VkSpecializationInfo inline writeSpecializationInfo(
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

void inline writeSpecializationInfo(
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

void inline writeSpecializationInfos(
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

// CIV
VkImageAspectFlags inline getAspectFlags(VkFormat format)
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

VkAccessFlags inline getAccessFlags(VkImageLayout layout)
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

VkPipelineStageFlags inline getStageFlags(VkImageLayout oldImageLayout)
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

inline VkVertexInputBindingDescription getVertexBindingDescription(uint32_t size, uint32_t binding = 0)
{
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding   = binding;
	vertexInputBindingDescription.stride    = size;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return vertexInputBindingDescription;
}

inline VkVertexInputBindingDescription getInstanceBindingDescription(uint32_t size, uint32_t binding = 1)
{
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding   = binding;
	vertexInputBindingDescription.stride    = size;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	return vertexInputBindingDescription;
}

inline std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, std::vector<VkFormat> formats, std::vector<uint32_t> offsets)
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
}        // namespace vka
