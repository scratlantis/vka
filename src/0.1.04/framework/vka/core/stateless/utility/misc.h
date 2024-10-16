#pragma once
#include "macros.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "../vk_types/misc.h"
namespace vka
{

// General helper functions/types

template <class integral>
constexpr integral alignUp(integral x, size_t a) noexcept
{
	return integral((x + (integral(a) - 1)) & ~integral(a - 1));
}

// from https://github.com/vcoda/magma
void *copyBinaryData(const void *src, std::size_t size) noexcept;

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

std::vector<uint8_t> getByteVector(void *data, size_t size);

template <class T>
inline uint32_t dataSize(std::vector<T> v)
{
	return static_cast<uint32_t>(v.size() * sizeof(T));
}

// Glm helper functions
glm::uvec3            getWorkGroupCount(const glm::uvec3 &workGroupSize, const glm::uvec3 &resolution);
std::vector<uint32_t> glm3VectorSizes();
std::vector<uint8_t>  getByteVector(const glm::uvec3 &in);

// Vulkan helper functions



constexpr void clamp(VkExtent2D &target, const VkExtent2D &minExtent, const VkExtent2D &maxExtent)
{
	target.width  = std::max(minExtent.width, std::min(maxExtent.width, target.width));
	target.height = std::max(minExtent.height, std::min(maxExtent.height, target.height));
}

uint32_t             findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void                 submit(std::vector<VkCommandBuffer> cmdBufs, VkQueue queue, const SubmitSynchronizationInfo syncInfo);
void                 submit(VkCommandBuffer cmdBuf, VkQueue queue, const SubmitSynchronizationInfo syncInfo);
constexpr VkExtent3D getExtent3D(const VkExtent2D extent)
{
	return {extent.width, extent.height, 1};
}

VkSpecializationInfo writeSpecializationInfo(
    const uint32_t                         mapEntriesCount,
    std::vector<VkSpecializationMapEntry> &mapEntries, const std::vector<uint32_t> &entrySizes, uint32_t &mapEntryOffset,
    const void *data, uint32_t &dataOffset);

void writeSpecializationInfo(
    const std::vector<uint32_t>           &entrySizes,
    const void                            *data,
    std::vector<VkSpecializationMapEntry> &mapEntries,
    VkSpecializationInfo                  &specInfo);

void writeSpecializationInfos(
    const std::vector<uint32_t>           &entryCounts,
    const std::vector<uint32_t>           &entrySizes,
    const void                            *data,
    std::vector<VkSpecializationMapEntry> &mapEntries,
    std::vector<VkSpecializationInfo>     &specInfos);

VkImageAspectFlags                             getAspectFlags(VkFormat format);
VkAccessFlags                                  getAccessFlags(VkImageLayout layout);
VkPipelineStageFlags                           getStageFlags(VkImageLayout oldImageLayout);
VkVertexInputBindingDescription                getVertexBindingDescription(uint32_t size, uint32_t binding = 0);
VkVertexInputBindingDescription                getInstanceBindingDescription(uint32_t size, uint32_t binding = 1);
VkTransformMatrixKHR                           glmToVk(glm::mat4 mat);
std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding, std::vector<VkFormat> formats, std::vector<uint32_t> offsets);
std::string                                    getGLSLFormat(VkFormat format);
glm::mat4                                      getMatrix(glm::vec3 pos, glm::vec3 rotDeg, float scale);
}        // namespace vka
