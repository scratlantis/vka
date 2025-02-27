#include "misc_utility.h"
#include <vka/globals.h>
#include <vka/core/resources/unique/misc.h>
#include "misc_utility.h"
namespace vka
{
VkQueryPool createQueryPool(IResourcePool *pPool, const VkQueryPoolCreateInfo &ci, Resource *&res)
	{
		VkQueryPool queryPool;
	    VK_CHECK(vkCreateQueryPool(gState.device.logical, &ci, nullptr, &queryPool));
	    res = new QueryPool_R(queryPool);
		res->track(pPool);
		return queryPool;
    }
}		// namespace vka