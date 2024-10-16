#pragma once
#include <vulkan/vulkan.h>

namespace vka
{

struct RayTracingShaderGroupCreateInfo_Empty : VkRayTracingShaderGroupCreateInfoKHR
{
	RayTracingShaderGroupCreateInfo_Empty();
};

struct ImageCreateInfo_Default : public VkImageCreateInfo
{
	ImageCreateInfo_Default(VkImageUsageFlags usageFlags, VkExtent2D extent, VkFormat format);

	ImageCreateInfo_Default(VkImageUsageFlags usageFlags, VkExtent3D extent, VkFormat format);
};


struct SamplerCreateInfo_Default : public VkSamplerCreateInfo
{
	SamplerCreateInfo_Default(float maxLodValue);
	SamplerCreateInfo_Default();
};

struct ImageViewCreateInfo_Default : public VkImageViewCreateInfo
{
	ImageViewCreateInfo_Default(const VkImage &vkImage, const VkFormat &vkFormat);
};

struct ImageViewCreateInfo3D_Default : public VkImageViewCreateInfo
{
	ImageViewCreateInfo3D_Default(const VkImage &vkImage, const VkFormat &vkFormat);
};

}        // namespace vka