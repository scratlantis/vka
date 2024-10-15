#pragma once
#include "types.h"
#include <vka/core/resources/IResourcePool.h>
namespace vka
{
Image createImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent2D extent);
Image createImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
Image createImage2D(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags);
Image createImage3D(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags);
}        // namespace vka