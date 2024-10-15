#include "device_properties.h"
#include <vka/globals.h>
namespace vka
{
VkPhysicalDeviceAccelerationStructurePropertiesKHR getAccelerationStructureProperties()
{
	VkPhysicalDeviceAccelerationStructurePropertiesKHR asProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR};
	VkPhysicalDeviceProperties2                        properties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
	properties.pNext = &asProperties;
	vkGetPhysicalDeviceProperties2(gState.device.physical, &properties);
	return asProperties;
}
}        // namespace vka