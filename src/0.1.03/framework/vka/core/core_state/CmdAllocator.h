#pragma once
#include <vka/core/stateless/utility/macros.h>
#include <vector>

namespace vka
{
class CmdAllocator
{
  public:
	CmdAllocator() = default;
	void init();
	bool createCmdBuffersUniversal(uint32_t queueIdx, VkCommandBufferLevel cmdBufLevel, uint32_t count, VkCommandBuffer &cmdBuf, VkCommandPool &cmdPool);
	bool createCmdBuffersCompute(uint32_t queueIdx, VkCommandBufferLevel cmdBufLevel, uint32_t count, VkCommandBuffer &cmdBuf, VkCommandPool &cmdPool);
	void destroy();
	DELETE_COPY_CONSTRUCTORS(CmdAllocator);

  private:
	std::vector<VkCommandPool> universalPools;
	std::vector<VkCommandPool> computePools;
};
}        // namespace vka