#include "Image.h"
#include <vka/globals.h>
#include <vka/core/stateless/vk_types/default_values.h>

namespace vka
{
void ImageVMA_R::free()
{
	gState.memAlloc.destroyImage(handle, allocation);
}
void ImageView_R::free()
{
	vkDestroyImageView(gState.device.logical, handle, nullptr);
}

void Image_R::createHandles()
{
	layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocationCreateInfo vmaAllocationCreateInfo{};
	vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VmaAllocation alloc;

	VkImageLayout targetInitialLayout = ci.initialLayout;
	ci.initialLayout                  = VK_IMAGE_LAYOUT_PREINITIALIZED;
	gState.memAlloc.createImage(&ci, &vmaAllocationCreateInfo, &handle, &alloc);
	ci.initialLayout = targetInitialLayout;

	res = new ImageVMA_R(handle, alloc);
	res->track(getPool());
	if (createView)
	{
		VkImageViewCreateInfo viewCI;
		if (ci.extent.depth > 1)
		{
			viewCI = ImageViewCreateInfo3D_Default(handle, format);
		}
		else
		{
			viewCI = ImageViewCreateInfo_Default(handle, format);
		}
		VK_CHECK(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &viewHandle));
		viewRes = new ImageView_R(viewHandle);
		viewRes->track(getPool());
	}
}

bool Image_R::recreate()
{
	// clang-format off
	if (ci.format == format &&
		ci.extent.depth == extent.depth && ci.extent.height == extent.height && ci.extent.width == extent.width
		&& ci.mipLevels == mipLevels
		&& ci.usage == usage)
	// clang-format on
	{
		return false;
	}
	ci.format    = format;
	ci.extent    = extent;
	ci.mipLevels = mipLevels;
	ci.usage     = usage;
	detachChildResources();
	createHandles();
	return true;
}

void Image_R::detachChildResources()
{
	if (res)
	{
		res->track(gState.frame->stack);
	}
	if (viewRes)
	{
		viewRes->track(gState.frame->stack);
	}
}

void Image_R::track(IResourcePool *pPool)
{
	if (viewRes)
	{
		viewRes->track(pPool);
	}
	if (res)
	{
		res->track(pPool);
	}
	Resource::track(pPool);
}

void Image_R::free()
{
}
hash_t Image_R::hash() const
{
	if (res)
		return res->hash() << VKA_RESOURCE_META_DATA_HASH_SHIFT;
	return 0;
}
VkImage SwapchainImage_R::getHandle() const
{
	return gState.io.images[gState.frame->frameIndex];
}
VkImageView SwapchainImage_R::getViewHandle() const
{
	return gState.io.imageViews[gState.frame->frameIndex];
}
VkFormat SwapchainImage_R::getFormat() const
{
	return gState.io.format;
}
VkExtent3D SwapchainImage_R::getExtent() const
{
	return {gState.io.extent.width, gState.io.extent.height, 1};
}

VkExtent2D SwapchainImage_R::getExtent2D() const
{
	return {gState.io.extent.width, gState.io.extent.height};
}
VkImageUsageFlags SwapchainImage_R::getUsage() const
{
	return gState.io.imageUsage;
}
VkImageLayout SwapchainImage_R::getLayout() const
{
	return gState.io.imageLayouts[gState.frame->frameIndex];
}
void SwapchainImage_R::setLayout(VkImageLayout layout)
{
	gState.io.imageLayouts[gState.frame->frameIndex] = layout;
}
hash_t SwapchainImage_R::hash() const
{
	return (hash_t) gState.io.images[gState.frame->frameIndex] << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}
}        // namespace vka