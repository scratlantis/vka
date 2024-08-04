#pragma once
#include <vka/core/stateless/utility/macros.h>
#include <vector>
#include <vka/core/stateless/containers/StructureChain.h>

namespace vka
{

struct DeviceCI
{
	std::string               applicationName;
	uint32_t                  universalQueueCount;
	uint32_t                  computeQueueCount;
	std::vector<const char *> enabledInstanceExtensions;
	std::vector<const char *> enabledDeviceExtensions;
	StructureChain            enabledFeatures;
};

class Device
{
  public:
	VkDevice             logical;
	VkPhysicalDevice     physical;
	VkInstance           instance;
	std::vector<VkQueue> universalQueues;
	int                  universalQueueFamily;
	std::vector<VkQueue> computeQueues;
	int                  computeQueueFamily;

	Device();
	void configure(DeviceCI &deviceCI);
	void destroy();
	void createInstance();
	void selectPhysicalDevice();
	void createLogicalDevice();

	DELETE_COPY_CONSTRUCTORS(Device);

  private:
	DeviceCI deviceCI;
};
}        // namespace vka