#pragma once
#include <vulkan/vulkan.h>
#include "../utility/misc.h"

namespace vka
{

struct RayTracingShaderGroupCreateInfo_Empty : VkRayTracingShaderGroupCreateInfoKHR
{
	RayTracingShaderGroupCreateInfo_Empty()
	{
		std::memset(this, 0, sizeof(RayTracingShaderGroupCreateInfo_Empty));
		sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		anyHitShader       = VK_SHADER_UNUSED_KHR;
		closestHitShader   = VK_SHADER_UNUSED_KHR;
		generalShader      = VK_SHADER_UNUSED_KHR;
		intersectionShader = VK_SHADER_UNUSED_KHR;
		type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	}
};

struct ImageCreateInfo_Default : public VkImageCreateInfo
{
	ImageCreateInfo_Default(VkImageUsageFlags usageFlags, VkExtent2D extent, VkFormat format)
	{
		std::memset(this, 0, sizeof(ImageCreateInfo_Default));
		this->sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		this->pNext                 = nullptr;
		this->flags                 = 0;
		this->imageType             = VK_IMAGE_TYPE_2D;
		this->format                = format;
		this->extent                = getExtent3D(extent);
		this->mipLevels             = 1;
		this->arrayLayers           = 1;
		this->samples               = VK_SAMPLE_COUNT_1_BIT;
		this->tiling                = VK_IMAGE_TILING_OPTIMAL;
		this->usage                 = usageFlags;
		this->sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
		this->queueFamilyIndexCount = 0;
		this->pQueueFamilyIndices   = nullptr;
		this->initialLayout         = VK_IMAGE_LAYOUT_PREINITIALIZED;
	}
};

struct ImageCreateInfo3D_Default : public VkImageCreateInfo
{
	ImageCreateInfo3D_Default(VkImageUsageFlags usageFlags, VkExtent3D extent, VkFormat format)
	{
		std::memset(this, 0, sizeof(ImageCreateInfo3D_Default));
		this->sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		this->pNext                 = nullptr;
		this->flags                 = 0;
		this->imageType             = VK_IMAGE_TYPE_3D;
		this->format                = format;
		this->extent                = extent;
		this->mipLevels             = 1;
		this->arrayLayers           = 1;
		this->samples               = VK_SAMPLE_COUNT_1_BIT;
		this->tiling                = VK_IMAGE_TILING_OPTIMAL;
		this->usage                 = usageFlags;
		this->sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
		this->queueFamilyIndexCount = 0;
		this->pQueueFamilyIndices   = nullptr;
		this->initialLayout         = VK_IMAGE_LAYOUT_PREINITIALIZED;
	}
};

struct SamplerCreateInfo_Default : public VkSamplerCreateInfo
{
	SamplerCreateInfo_Default(float maxLodValue)
	{
		std::memset(this, 0, sizeof(SamplerCreateInfo_Default));
		sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		pNext                   = nullptr;
		flags                   = 0;
		magFilter               = VK_FILTER_LINEAR;
		minFilter               = VK_FILTER_LINEAR;
		mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		mipLodBias              = 0.0;
		anisotropyEnable        = VK_FALSE;
		maxAnisotropy           = 1.0;
		compareEnable           = VK_FALSE;
		compareOp               = VK_COMPARE_OP_NEVER;
		minLod                  = 0.0;
		maxLod                  = maxLodValue;
		borderColor             = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		unnormalizedCoordinates = VK_FALSE;
	}
	SamplerCreateInfo_Default() :
	    SamplerCreateInfo_Default(0.0f)
	{}
};

struct ImageViewCreateInfo_Default : public VkImageViewCreateInfo
{
	ImageViewCreateInfo_Default(const VkImage &vkImage, const VkFormat &vkFormat)
	{
		std::memset(this, 0, sizeof(ImageViewCreateInfo_Default));
		sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		subresourceRange.baseMipLevel   = 0;
		subresourceRange.levelCount     = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount     = 1;
		image                           = vkImage;
		format                          = vkFormat;
		subresourceRange.aspectMask     = getAspectFlags(vkFormat);
	}
};

struct ImageViewCreateInfo3D_Default : public VkImageViewCreateInfo
{
	ImageViewCreateInfo3D_Default(const VkImage &vkImage, const VkFormat &vkFormat)
	{
		std::memset(this, 0, sizeof(ImageViewCreateInfo3D_Default));
		sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewType                        = VK_IMAGE_VIEW_TYPE_3D;
		components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		subresourceRange.baseMipLevel   = 0;
		subresourceRange.levelCount     = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount     = 1;
		image                           = vkImage;
		format                          = vkFormat;
		subresourceRange.aspectMask     = getAspectFlags(vkFormat);
	}
};

}        // namespace vka