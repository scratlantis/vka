#include "image_utility.h"
#include <vka/core/stateless/vk_types/default_values.h>
namespace vka
{
	Image createImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent2D extent)
	{
		VkImageCreateInfo ci  = ImageCreateInfo_Default(usageFlags, extent, format);
		Image          img = new Image_R(pPool, ci, true);
		img->createHandles();
		return img;
	}

	Image createImage(IResourcePool *pPool, VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
	{
		VkImageCreateInfo ci  = ImageCreateInfo3D_Default(usageFlags, extent, format);
		Image          img = new Image_R(pPool, ci, true);
		img->createHandles();
		return img;
	}
}		// namespace vka