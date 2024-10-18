#pragma once
#include <unordered_set>
namespace vka
{
class Resource;
class Image_R;
class Buffer_R;
class CmdBuffer_R;
class Mappable_T;
class IResourcePool
{
  public:
	virtual bool add(Resource *resource)            = 0;
	virtual bool remove(Resource *resource)         = 0;

	virtual std::unordered_set<Image_R *>::iterator getImagesBegin() = 0;
	virtual std::unordered_set<Image_R *>::iterator getImagesEnd() = 0;

	virtual std::unordered_set<Buffer_R *>::iterator getBuffersBegin() = 0;
	virtual std::unordered_set<Buffer_R *>::iterator getBuffersEnd()   = 0;

	virtual void clear()                            = 0;
};
}        // namespace vka