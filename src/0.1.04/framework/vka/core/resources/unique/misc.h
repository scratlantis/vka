#pragma once
#include "../Resource.h"
#include <vka/core/stateless/utility/constants.h>
#include <vka/core/stateless/vk_types/misc.h>

namespace vka
{

class QueryPool_R : public Resource_T<VkQueryPool>
{
  public:
	QueryPool_R(VkQueryPool handle) :
	    Resource_T<VkQueryPool>(handle)
	{
	}
	void free() override;
};
}