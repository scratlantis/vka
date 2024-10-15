#pragma once
#include <vka/core/core_utility/types.h>

namespace vka
{

class ImGuiWrapper
{
  public:
	ImGuiWrapper(){};
	~ImGuiWrapper(){};

	void init(VkRenderPass renderPass, uint32_t subpassIdx);
	void upload();
	void render(CmdBuffer cmdBuf);
	void newFrame();
	void destroy();

  private:
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
};
}        // namespace vka