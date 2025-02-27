#pragma once
#include <unordered_map>
#include <vka/core/core_common.h>
#include <imgui.h>
namespace vka
{
class ImGuiTextureIDCache
{
	std::unordered_map<VkImage, VkDescriptorSet> map;
	std::unordered_map<VkDescriptorSet, Image> invMap;
	IResourcePool                     *pPool;


	std::vector<std::vector<ImTextureID>> garbageList;
	bool        freeGarbage(ImTextureID id);

  public:
	ImGuiTextureIDCache(IResourcePool *pPool);
	void        clear();
	ImTextureID fetch(Image img);
	void        garbage(ImTextureID id);
	void        processGarbage();
};
}        // namespace vka