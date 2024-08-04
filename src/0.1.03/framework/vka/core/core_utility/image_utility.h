#pragma once
#include "types.h"
#include <vka/core/resources/IResourcePool.h>
namespace vka
{
Image createImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent2D extent);
Image createImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
}        // namespace vka