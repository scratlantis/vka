#include "ResourcePool.h"
#include <vka/core/resources/unique/Image.h>
namespace vka
{
bool ResourcePool::add(Resource *resource)
{
	return resources.insert(resource).second;
}
bool ResourcePool::add(Image_R *img)
{
	return images.insert(img).second;
}

bool ResourcePool::remove(Resource *resource)
{
	return resources.erase(resource);
}

bool ResourcePool::remove(Image_R *img)
{
	return images.erase(img);
}

std::unordered_set<Image_R *>::iterator ResourcePool::getImagesBegin()
{
	return images.begin();
}

std::unordered_set<Image_R *>::iterator ResourcePool::getImagesEnd()
{
	return images.end();
}

void ResourcePool::clear()
{
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		(*it)->free();
		delete *it;
	}
	resources.clear();
	for (auto it = images.begin(); it != images.end(); ++it)
	{
		(*it)->free();
		delete *it;
	}
	images.clear();
}
}        // namespace vka