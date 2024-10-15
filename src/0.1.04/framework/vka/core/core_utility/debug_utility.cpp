#include "debug_utility.h"
#include <vka/globals.h>
namespace vka
{
 void setDebugMarker(Buffer buffer, const char *name)
{
	 LOAD_CMD_VK_DEVICE(vkSetDebugUtilsObjectNameEXT, gState.device.logical);

	const VkDebugUtilsObjectNameInfoEXT objNameInfo =
	    {
	        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
	        NULL,                  // pNext
	        VK_OBJECT_TYPE_BUFFER,                  // objectType
	         (uint64_t) buffer->getHandle(),        // object eg: VK_OBJECT_TYPE_IMAGE,
	        name,                  // pObjectName
	    };
	pvkSetDebugUtilsObjectNameEXT(gState.device.logical, &objNameInfo);
}
}