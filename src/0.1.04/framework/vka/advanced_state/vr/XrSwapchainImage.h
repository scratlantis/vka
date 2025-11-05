#pragma once
#include <vka/core/core_common.h>

namespace vka
{

	enum XrEye
	{
		XR_EYE_LEFT  = 0,
		XR_EYE_RIGHT = 1,
	};
class XrSwapchainImage_R : public SwapchainImage_R
{
  protected:
	XrSwapchainImage_R(const Image_R &rhs) :
	    SwapchainImage_R(rhs){};
	XrEye eye = XR_EYE_LEFT;
  public:
	XrSwapchainImage_R() = default;
	XrSwapchainImage_R(XrEye eye) : eye(eye){}

	VkImage           getHandle() const override;
	VkImageView       getViewHandle() const override;
	VkFormat          getFormat() const override;
	VkExtent3D        getExtent() const override;
	VkExtent2D        getExtent2D() const override;
	VkImageUsageFlags getUsage() const override;
	VkImageLayout     getLayout() const override;
	void              setLayout(VkImageLayout layout) override;

  public:
	
	hash_t hash() const override;
};
}