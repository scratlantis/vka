#pragma once
#include <vka/core/core_utility/types.h>
namespace vka
{
void             swapBuffers(std::vector<CmdBuffer> cmdBufs);
vka::VkRect2D_OP getScissorRect(float x, float y, float width, float height);
Image            createSwapchainAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout);
Image            createSwapchainAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout, float widthCoef, float heightCoef);
Image            getSwapchainImage();
void             clearShaderCache();
void             waitIdle();

}		// namespace vka