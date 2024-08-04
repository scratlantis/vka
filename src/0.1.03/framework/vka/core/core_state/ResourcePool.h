#pragma once
#include <vka/core/resources/Resource.h>
#include <unordered_set>
#include <vka/core/resources/unique/Image.h>
namespace vka
{

class ResourcePool : public IResourcePool
{
  private:
	std::unordered_set<Resource *> resources;
	std::unordered_set<Image_R *>  images;

  public:
	bool add(Resource *resource) override;
	bool add(Image_R *img) override;
	bool remove(Resource *resource) override;
	bool remove(Image_R *img) override;
	std::unordered_set<Image_R *>::iterator getImagesBegin() override;
	std::unordered_set<Image_R *>::iterator getImagesEnd() override;
	ResourcePool(){};
	~ResourcePool(){};
	void clear() override;
	DELETE_COPY_CONSTRUCTORS(ResourcePool);
};


}        // namespace vka
