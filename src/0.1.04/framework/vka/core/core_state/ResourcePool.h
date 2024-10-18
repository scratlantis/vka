#pragma once
#include <vka/core/resources/Resource.h>
#include <vka/core/resources/unique/Buffer.h>
#include <unordered_set>
#include <vka/core/resources/unique/Image.h>
namespace vka
{

class ResourcePool : public IResourcePool
{
  private:
	std::unordered_set<Buffer_R *>     buffers;
	std::unordered_set<Resource *>   resources;
	std::unordered_set<Image_R *>    images;
	std::unordered_set<Mappable_T *> mappables;

  public:
	bool add(Resource *resource) override;
	bool remove(Resource *resource) override;

	std::unordered_set<Buffer_R *>::iterator getBuffersBegin() override;
	std::unordered_set<Buffer_R *>::iterator getBuffersEnd() override;

	std::unordered_set<Image_R *>::iterator getImagesBegin() override;
	std::unordered_set<Image_R *>::iterator getImagesEnd() override;
	ResourcePool(){};
	~ResourcePool(){};
	void clear() override;
	DELETE_COPY_CONSTRUCTORS(ResourcePool);
};


}        // namespace vka
