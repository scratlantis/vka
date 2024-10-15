#include "ResourcePool.h"
#include <vka/core/resources/unique/Image.h>
namespace vka
{
bool ResourcePool::add(Resource *resource)
{
	if (resource->type() == RESOURCE_TYPE_MAPPAPLE)
	{
		auto resource_ = static_cast<Mappable_T*>(resource);
		return mappables.insert(resource_).second;
	}
	if (resource->type() == RESOURCE_TYPE_IMAGE)
	{
		auto resource_ = static_cast<Image_R *>(resource);
		return images.insert(resource_).second;
	}
	return resources.insert(resource).second;
}

bool ResourcePool::remove(Resource *resource)
{
	return resources.erase(resource);
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
	for (auto it = mappables.begin(); it != mappables.end(); ++it)
	{
		(*it)->free();
		delete *it;
	}
	mappables.clear();
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