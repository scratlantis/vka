#pragma once
#include <vector>
#include <vka/core/core_state/Device.h>
#include <set>
#include <string>


namespace vka
{

class VrDevice
{
  public:
	VkDevice                 logical;
	VkPhysicalDevice         physical;
	VkInstance               instance;
	VkDebugUtilsMessengerEXT printfDebugMessenger;
	VkDebugUtilsMessengerEXT validationDebugMessenger;
	std::vector<VkQueue>     universalQueues;
	int                      universalQueueFamily;
	std::vector<VkQueue>     computeQueues;
	int                      computeQueueFamily;

	VrDevice() = default;
	~VrDevice() = default;
	void configure(DeviceCI &deviceCI);
	void destroy();
	void createInstance();
	void selectPhysicalDevice();
	void createLogicalDevice();

	DELETE_COPY_CONSTRUCTORS(VrDevice);

  protected:
	DeviceCI deviceCI;
};
}        // namespace vka