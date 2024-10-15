#pragma once
#include <unordered_map>
#include <vka/core/core_common.h>
#include <imgui.h>
namespace vka
{
class ImGuiTextureIDCache
{
	std::unordered_map<VkImage, VkDescriptorSet> map;
	IResourcePool                     *pPool;

  public:
	ImGuiTextureIDCache(IResourcePool *pPool) :
	    pPool(pPool){};
	void clear();
	ImTextureID fetch(Image img);
};
}        // namespace vka