#include "ImGuiTextureIDCache.h"
#include <imgui_impl_vulkan.h>
#include <vka/globals.h>
namespace vka
{
void ImGuiTextureIDCache::clear()
{
	for (auto &pair : map)
	{
		ImGui_ImplVulkan_RemoveTexture(pair.second);
	}
	map.clear();
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
		return descSet;
	}
}


}        // namespace vka