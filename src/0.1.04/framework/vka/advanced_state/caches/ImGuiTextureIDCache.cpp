#include "ImGuiTextureIDCache.h"
#include <imgui_impl_vulkan.h>
#include <vka/globals.h>
namespace vka
{
void ImGuiTextureIDCache::garbage(ImTextureID id)
{
	garbageList[gState.frame->frameIndex].push_back(id);
}
ImGuiTextureIDCache::ImGuiTextureIDCache(IResourcePool *pPool) : pPool(pPool)
{
	garbageList.resize(gState.io.imageCount);
}
void ImGuiTextureIDCache::clear()
{
	for (auto &pair : map)
	{
		ImGui_ImplVulkan_RemoveTexture(pair.second);
	}
	map.clear();
	invMap.clear();
}
ImTextureID ImGuiTextureIDCache::fetch(Image img)
{
	VkImage key = img->getHandle();
	if (map.find(key) != map.end())
	{
		return map[key];
	}
	else
	{
		SamplerDefinition smplInfo = SamplerDefinition();
		smplInfo.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		smplInfo.magFilter     = VK_FILTER_LINEAR;
		smplInfo.minFilter     = VK_FILTER_LINEAR;
		smplInfo.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		smplInfo.addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		smplInfo.addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		smplInfo.addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		smplInfo.minLod        = -1000;
		smplInfo.maxLod        = 1000;
		smplInfo.maxAnisotropy = 1.0f;
		VkDescriptorSet descSet    = ImGui_ImplVulkan_AddTexture(gState.cache->fetch(smplInfo), img->getViewHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		map[key] = descSet;
		invMap[descSet] = img;
		return descSet;
	}
}
void ImGuiTextureIDCache::processGarbage()
{
	for (auto &garbage : garbageList[gState.frame->frameIndex])
	{
		freeGarbage(garbage);
	}
	garbageList[gState.frame->frameIndex].clear();
}

bool ImGuiTextureIDCache::freeGarbage(ImTextureID id)
{
	VkDescriptorSet descSet = reinterpret_cast<VkDescriptorSet>(id);
	if (invMap.find(descSet) != invMap.end())
	{
		Image img = invMap[descSet];
		map.erase(img->getHandle());
		ImGui_ImplVulkan_RemoveTexture(descSet);
		img->garbageCollect();
	}
	return false;
}


}        // namespace vka