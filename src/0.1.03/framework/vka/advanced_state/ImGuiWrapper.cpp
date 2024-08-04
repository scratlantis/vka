
#include "ImGuiWrapper.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vka/globals.h>

namespace vka
{
void ImGuiWrapper::init(VkRenderPass renderPass, uint32_t subpassIdx)
{
	// GLFW specific implementation
	gState.io.getWindow()->initImGui();

	// Create Descriptor Pool
	VkDescriptorPoolSize guiPoolSizes[] =
	    {
	        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
	        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
	        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
	        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
	        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
	        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
	        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
	        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
	        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
	        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
	        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

	VkDescriptorPoolCreateInfo guiPoolInfo = {};
	guiPoolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	guiPoolInfo.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	guiPoolInfo.maxSets                    = 1000 * IM_ARRAYSIZE(guiPoolSizes);
	guiPoolInfo.poolSizeCount              = (uint32_t) IM_ARRAYSIZE(guiPoolSizes);
	guiPoolInfo.pPoolSizes                 = guiPoolSizes;

	VK_CHECK(vkCreateDescriptorPool(gState.device.logical, &guiPoolInfo, nullptr, &descriptorPool));

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance                  = gState.device.instance;
	initInfo.PhysicalDevice            = gState.device.physical;
	initInfo.Device                    = gState.device.logical;
	initInfo.QueueFamily               = gState.device.universalQueueFamily;
	initInfo.Queue                     = gState.device.universalQueues[0];
	initInfo.DescriptorPool            = descriptorPool;
	initInfo.Subpass                   = subpassIdx;
	initInfo.PipelineCache             = VK_NULL_HANDLE;        // we do not need those
	initInfo.MinImageCount             = 2;
	initInfo.ImageCount                = gState.io.imageCount;
	initInfo.Allocator                 = VK_NULL_HANDLE;
	initInfo.CheckVkResultFn           = VK_NULL_HANDLE;
	initInfo.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;
	ImGui_ImplVulkan_Init(&initInfo, renderPass);
}

void ImGuiWrapper::upload(CmdBuffer cmdBuf)
{
	ImGui_ImplVulkan_CreateFontsTexture(cmdBuf->getHandle());
}

void ImGuiWrapper::freeStaging()
{
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiWrapper::newFrame()
{
	gState.io.getWindow()->newFrameImGui();
	ImGui::NewFrame();
}

void ImGuiWrapper::render(CmdBuffer cmdBuf)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf->getHandle());
}

void ImGuiWrapper::destroy()
{
	vkDestroyDescriptorPool(gState.device.logical, descriptorPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
}
}        // namespace vka