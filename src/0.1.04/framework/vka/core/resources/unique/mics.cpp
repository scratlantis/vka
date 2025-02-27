#include "misc.h"
#include <vka/globals.h>
namespace vka
{
void vka::QueryPool_R::free()
{
	vkDestroyQueryPool(gState.device.logical, handle, nullptr);
}
}