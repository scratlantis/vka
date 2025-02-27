#include "ResourcePool.h"
#include <vka/core/resources/unique/Image.h>
namespace vka
{
bool ResourcePool::add(Resource *resource)
{
	switch (resource->type())
	{
		case RESOURCE_TYPE_MAPPAPLE:
		{
			auto resource_ = static_cast<Mappable_T *>(resource);
			return mappables.insert(resource_).second;
		}
		case RESOURCE_TYPE_IMAGE:
		{
			auto resource_ = static_cast<Image_R *>(resource);
			return images.insert(resource_).second;
		}
		case RESOURCE_TYPE_BUFFER:
		{
			auto resource_ = static_cast<Buffer_R *>(resource);
			return buffers.insert(resource_).second;
		}
		default:
			return resources.insert(resource).second;
	}
}

bool ResourcePool::remove(Resource *resource)
{
	switch (resource->type())
	{
		case RESOURCE_TYPE_MAPPAPLE:
		{
			auto resource_ = static_cast<Mappable_T*>(resource);
			return mappables.erase(resource_);
		}
		case RESOURCE_TYPE_IMAGE:
		{
			auto resource_ = static_cast<Image_R *>(resource);
			return images.erase(resource_);
		}
		case RESOURCE_TYPE_BUFFER:
		{
			auto resource_ = static_cast<Buffer_R *>(resource);
			return buffers.erase(resource_);
		}
		default:
			return resources.erase(resource);
	}
}

std::unordered_set<Buffer_R *>::iterator ResourcePool::getBuffersBegin()
{
	return buffers.begin();
}

std::unordered_set<Buffer_R *>::iterator ResourcePool::getBuffersEnd()
{
	return buffers.end();
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
	for (auto it = buffers.begin(); it != buffers.end(); ++it)
	{
		(*it)->free();
		delete *it;
	}
	buffers.clear();
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
void ResourcePool::dump(IResourcePool *otherPool)
{
	for (auto it = mappables.begin(); it != mappables.end(); ++it)
	{
		otherPool->add(*it);
	}
	mappables.clear();
	for (auto it = buffers.begin(); it != buffers.end(); ++it)
	{
		otherPool->add(*it);
	}
	buffers.clear();
	for (auto it = resources.begin(); it != resources.end(); ++it)
	{
		otherPool->add(*it);
	}
	resources.clear();
	for (auto it = images.begin(); it != images.end(); ++it)
	{
		otherPool->add(*it);
	}
	images.clear();
}
}        // namespace vka