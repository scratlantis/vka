#pragma once
#include "XrSwapchainImage.h"
#include <vka/globals.h>
#include "xr_common.h"

namespace vka
{
VkImage XrSwapchainImage_R::getHandle() const
{
	if (eye == XR_EYE_LEFT)
	{
		return gState.xrHeadset.imagesLeft[gState.frame->frameIndex];
	}
	else
	{
		return gState.xrHeadset.imagesRight[gState.frame->frameIndex];
	}
}
VkImageView XrSwapchainImage_R::getViewHandle() const
{
	if ( eye == XR_EYE_LEFT)
	{
		return gState.xrHeadset.imageViewsLeft[gState.frame->frameIndex];
	}
	else
	{
		return gState.xrHeadset.imageViewsRight[gState.frame->frameIndex];
	}
}
VkFormat XrSwapchainImage_R::getFormat() const
{
	return xr::colorFormat;
}
VkExtent3D XrSwapchainImage_R::getExtent() const
{
	return {gState.xrHeadset.eyeResolution.width, gState.xrHeadset.eyeResolution.height, 1};
}

VkExtent2D XrSwapchainImage_R::getExtent2D() const
{
	return {gState.xrHeadset.eyeResolution.width, gState.xrHeadset.eyeResolution.height};
}
VkImageUsageFlags XrSwapchainImage_R::getUsage() const
{
	return gState.io.imageUsage;
}
VkImageLayout XrSwapchainImage_R::getLayout() const
{
	if ( eye == XR_EYE_LEFT)
	{
		return gState.xrHeadset.imageLayoutsLeft[gState.frame->frameIndex];
	}
	else
	{
		return gState.xrHeadset.imageLayoutsRight[gState.frame->frameIndex];
	}
}
void XrSwapchainImage_R::setLayout(VkImageLayout layout)
{
	if ( eye == XR_EYE_LEFT)
	{
		gState.xrHeadset.imageLayoutsLeft[gState.frame->frameIndex] = layout;
	}
	else
	{
		gState.xrHeadset.imageLayoutsRight[gState.frame->frameIndex] = layout;
	}
}
hash_t XrSwapchainImage_R::hash() const
{
	if ( eye == XR_EYE_LEFT)
	{
		return (hash_t) gState.xrHeadset.imagesLeft[gState.frame->frameIndex] << VKA_RESOURCE_META_DATA_HASH_SHIFT;
	}
	else
	{
		return (hash_t) gState.xrHeadset.imagesRight[gState.frame->frameIndex] << VKA_RESOURCE_META_DATA_HASH_SHIFT;
	}
}
}