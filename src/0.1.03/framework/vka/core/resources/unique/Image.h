#pragma once
#include "../Resource.h"
#include <vka/core/stateless/utility/constants.h>
#include <vka/core/stateless/vk_types/misc.h>
#include <vk_mem_alloc.h>
namespace vka
{

class ImageVMA_R : public Resource_T<VkImage>
{
  public:
	ImageVMA_R(VkImage handle, VmaAllocation allocation) :
	    Resource_T<VkImage>(handle)
	{
		this->allocation = allocation;
	}
	void free() override;

  private:
	VmaAllocation allocation;
};

class ImageView_R : public Resource_T<VkImageView>
{
  public:
	ImageView_R(VkImageView handle) :
	    Resource_T<VkImageView>(handle)
	{}
	void free() override;
};

class Image_R : public Resource_T<VkImage>
{
  protected:
	Resource *res     = nullptr;
	Resource *viewRes = nullptr;
	bool      createView;

	Image_R() = default;
  private:
	VkImageView viewHandle = VK_NULL_HANDLE;

	// Current configuration
	VkImageCreateInfo ci;

	// Relative area
	float relativeWidth  = 1.0f;
	float relativeHeight = 1.0f;

	// Overrides
	VkFormat          format;
	VkExtent3D        extent;
	uint32_t          mipLevels;
	VkImageUsageFlags usage;

	VkImageLayout layout;
	ClearValue	clearValue;

  public:

	virtual ResourceType type() const
	{
		return RESOURCE_TYPE_IMAGE;
	}

	VkDeviceSize getMemorySize() const
	{
		VkDeviceSize size = extent.width * extent.height * extent.depth;
		size *= mipLevels;
		size *= cVkFormatTable.at(format).size;
		return size;
	}

	virtual VkImageView getViewHandle() const
	{
		return viewHandle;
	}
	virtual VkFormat getFormat() const
	{
		return ci.format;
	}
	virtual VkExtent3D getExtent() const
	{
		return ci.extent;
	}
	virtual VkExtent2D getExtent2D() const
	{
		return {ci.extent.width, ci.extent.height};
	}
	virtual uint32_t getMipLevels() const
	{
		return ci.mipLevels;
	}
	virtual VkImageUsageFlags getUsage() const
	{
		return ci.usage;
	}
	virtual VkImageLayout getLayout() const
	{
		return layout;
	}
	virtual VkImageLayout getInitialLayout() const
	{
		return ci.initialLayout;
	}
	virtual void setLayout(VkImageLayout layout)
	{
		this->layout = layout;
	}
	Image_R(IResourcePool *pPool, VkImageCreateInfo imgCI, bool createView)
	{
		ci               = imgCI;
		format           = ci.format;
		extent           = ci.extent;
		mipLevels        = ci.mipLevels;
		usage            = ci.usage;
		layout           = VK_IMAGE_LAYOUT_UNDEFINED;
		this->createView = createView;
		track(pPool);
	}
	Image_R(IResourcePool *pPool, VkImageCreateInfo imgCI, bool createView, float widthCoef, float heightCoef)
	{
		ci               = imgCI;
		format           = ci.format;
		extent           = ci.extent;
		mipLevels        = ci.mipLevels;
		usage            = ci.usage;
		layout           = VK_IMAGE_LAYOUT_UNDEFINED;
		this->createView = createView;
		track(pPool);

		relativeWidth  = widthCoef;
		relativeHeight = heightCoef;
		extent.width   = static_cast<uint32_t>(extent.width * widthCoef);
		extent.height  = static_cast<uint32_t>(extent.height * heightCoef);
	}

	~Image_R()
	{
		free();
	}

  public:
	Image_R(const Image_R &rhs)
	{
		// No ownership, no tracking
		res     = nullptr;
		viewRes = nullptr;

		ci = rhs.ci;
		createView = rhs.createView;

		handle     = rhs.handle;
		viewHandle = rhs.viewHandle;

		format    = rhs.format;
		extent    = rhs.extent;
		mipLevels = rhs.mipLevels;
		usage     = rhs.usage;
		layout    = rhs.layout;
	}

	virtual void changeFormat(VkFormat format)
	{
		this->format = format;
	}
	virtual void changeExtent(VkExtent3D extent)
	{
		this->extent = extent;
		this->extent.width  = static_cast<uint32_t>(extent.width * relativeWidth);
		this->extent.height = static_cast<uint32_t>(extent.height * relativeHeight);
	}
	virtual void changeMipLevels(uint32_t mipLevels)
	{
		this->mipLevels = mipLevels;
	}
	virtual void changeUsage(VkImageUsageFlags usage)
	{
		this->usage = usage;
	}

	virtual void    createHandles();
	virtual Image_R recreate();
	virtual void   detachChildResources();
	virtual void   track(IResourcePool *pPool) override;
	virtual void   free() override;
	virtual hash_t hash() const override;
	void setClearValue(ClearValue clearValue)
	{
		this->clearValue = clearValue;
	}
	void removeClearValue()
	{
		clearValue = ClearValue::none();
	}
	ClearValue getClearValue() const
	{
		return clearValue;
	}

};

class SwapchainImage_R : public Image_R
{
  protected:
	SwapchainImage_R(const Image_R &rhs) :
	    Image_R(rhs){};
  public:
	SwapchainImage_R() = default;

	VkImage     getHandle() const override;
	VkImageView getViewHandle() const override;
	VkFormat    getFormat() const override;
	VkExtent3D  getExtent() const override;
	VkExtent2D  getExtent2D() const override;
	uint32_t    getMipLevels() const override
	{
		return 1;
	}
	VkImageUsageFlags getUsage() const override;
	VkImageLayout     getLayout() const override;
	void              setLayout(VkImageLayout layout) override;

	void changeFormat(VkFormat format) override
	{
		printVka("Attempted to change swapchain image");
		DEBUG_BREAK;
	}
	void changeExtent(VkExtent3D extent) override
	{
		printVka("Attempted to change swapchain image");
		DEBUG_BREAK;
	}
	void changeMipLevels(uint32_t mipLevels) override
	{
		printVka("Attempted to change swapchain image");
		DEBUG_BREAK;
	}
	void changeUsage(VkImageUsageFlags usage) override
	{
		printVka("Attempted to change swapchain image");
		DEBUG_BREAK;
	}

  public:
	void createHandles() override
	{
		printVka("Attempted to create handles for swapchain image");
		DEBUG_BREAK;
	};
	Image_R recreate() override
	{
		printVka("Attempted to recreate swapchain image");
		DEBUG_BREAK;
		return *this;
	};
	void detachChildResources() override{};
	void track(IResourcePool *pPool) override
	{
		printVka("Attempted to track swapchain image");
		DEBUG_BREAK;
	};
	void free() override
	{
		printVka("Attempted to free swapchain image");
		DEBUG_BREAK;
	};
	hash_t hash() const override;
};

}        // namespace vka
