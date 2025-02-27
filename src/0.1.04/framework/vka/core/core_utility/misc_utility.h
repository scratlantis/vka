#pragma once
#include "types.h"
#include <vka/core/resources/IResourcePool.h>

namespace vka
{
VkQueryPool createQueryPool(IResourcePool *pPool, const VkQueryPoolCreateInfo &ci, Resource *&res);
}