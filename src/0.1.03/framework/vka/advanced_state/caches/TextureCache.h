#pragma once
#include "stb_image.h"
#include <unordered_map>
#include <vka/core/core_utility/types.h>
namespace vka
{
struct ImageData
{
	int   width;
	int   height;
	int   channels;
	int   quantisation;
	void *data;

	size_t size() const
	{
		return width * height * channels * quantisation;
	}
};

class TextureCache
{
	std::unordered_map<std::string, Image> map;
	std::string                               texturePath;
	IResourcePool                            *pPool;

  public:
	TextureCache(IResourcePool *pPool, std::string texturePath) :
	    texturePath(texturePath), pPool(pPool)
	{}

	void     clear();
	Image fetch(CmdBuffer cmdBuf, std::string path, VkFormat format, VkImageUsageFlags usage, VkImageLayout layout);
};

}        // namespace vka