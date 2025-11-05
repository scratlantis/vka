#pragma once
#include <vector>
#include <vka/core/core_state/Device.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <string>


namespace vka
{
class XrHeadset
{
  public:
	XrHeadset() = default;
	~XrHeadset() = default;

	bool                   exitRequested = false;
	size_t                 eyeCount      = 0u;
	std::vector<glm::mat4> eyeViewMatrices;
	std::vector<glm::mat4> eyeProjectionMatrices;

	XrSession      session      = nullptr;
	XrSessionState sessionState = XR_SESSION_STATE_UNKNOWN;
	XrSpace        space        = nullptr;
	XrFrameState   frameState   = {};
	XrViewState    viewState    = {};

	std::vector<XrViewConfigurationView>          eyeImageInfos;
	std::vector<XrView>                           eyePoses;
	std::vector<XrCompositionLayerProjectionView> eyeRenderInfos;
	VkExtent2D           eyeResolution;

	std::vector<XrSwapchain>   swapchains;

	std::vector<VkImage>       imagesLeft;
	std::vector<VkImage>       imagesRight;

	std::vector<VkImageView>   imageViewsLeft;
	std::vector<VkImageView>   imageViewsRight;

	std::vector<VkImageLayout> imageLayoutsLeft;
	std::vector<VkImageLayout> imageLayoutsRight;


	VkExtent2D getEyeResolution(size_t eyeIndex) const;

	void init();

	bool beginSession() const;

	bool endSession() const;

	void destroy();




	enum class BeginFrameResult
	{
		Error,              // An error occurred
		RenderFully,        // Render this frame normally
		SkipRender,         // Process this frame but skip rendering
		SkipFully           // Skip processing this frame entirely without ending it
	};
	BeginFrameResult beginFrame(uint32_t &swapchainImageIndexLeft, uint32_t &swapchainImageIndexRight);

	void endFrame() const;

	DELETE_COPY_CONSTRUCTORS(XrHeadset)

  private:
};




}