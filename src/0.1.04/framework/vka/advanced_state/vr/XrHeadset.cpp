#include "XrHeadset.h"
#include "xr_common.h"
#include <vka/globals.h>



constexpr XrReferenceSpaceType spaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
namespace vka
{

VkExtent2D XrHeadset::getEyeResolution(size_t eyeIndex) const
{
	const XrViewConfigurationView &eyeInfo = eyeImageInfos.at(eyeIndex);
	return {eyeInfo.recommendedImageRectWidth, eyeInfo.recommendedImageRectHeight};
}

void XrHeadset::init()
{
	swapchainImageFormat = xr::colorFormat;
	// Create a session with Vulkan graphics binding
	XrGraphicsBindingVulkan2KHR graphicsBinding{XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR};
	graphicsBinding.device           = gState.device.logical;
	graphicsBinding.instance         = gState.device.instance;
	graphicsBinding.physicalDevice   = gState.device.physical;
	graphicsBinding.queueFamilyIndex = gState.device.universalQueueFamily;
	graphicsBinding.queueIndex       = 0u;

	XrSessionCreateInfo sessionCreateInfo{XR_TYPE_SESSION_CREATE_INFO};
	sessionCreateInfo.next     = &graphicsBinding;
	sessionCreateInfo.systemId = gState.xrContext.xrSystemId;
	VKXR_CHECK(xrCreateSession(gState.xrContext.xrInstance, &sessionCreateInfo, &session));

	// Create a play space
	XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
	referenceSpaceCreateInfo.referenceSpaceType   = spaceType;
	referenceSpaceCreateInfo.poseInReferenceSpace = xr::makeIdentity();
	VKXR_CHECK(xrCreateReferenceSpace(session, &referenceSpaceCreateInfo, &space));

	VKXR_CHECK(xrEnumerateViewConfigurationViews(gState.xrContext.xrInstance, gState.xrContext.xrSystemId, xr::viewType, 0u,
	                                  reinterpret_cast<uint32_t *>(&eyeCount), nullptr));
	// Get the eye image info per eye
	eyeImageInfos.resize(eyeCount);
	for (XrViewConfigurationView &eyeInfo : eyeImageInfos)
	{
		eyeInfo.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
		eyeInfo.next = nullptr;
	}

	VKXR_CHECK(xrEnumerateViewConfigurationViews(gState.xrContext.xrInstance, gState.xrContext.xrSystemId, xr::viewType,
	                                  static_cast<uint32_t>(eyeImageInfos.size()), reinterpret_cast<uint32_t *>(&eyeCount),
	                                  eyeImageInfos.data()));

	// Allocate the eye poses
	eyePoses.resize(eyeCount);
	for (XrView &eyePose : eyePoses)
	{
		eyePose.type = XR_TYPE_VIEW;
		eyePose.next = nullptr;
	}
	// Verify that the desired color format is supported
	{
		uint32_t formatCount = 0u;
		VKXR_CHECK(xrEnumerateSwapchainFormats(session, 0u, &formatCount, nullptr));
		std::vector<int64_t> formats(formatCount);
		VKXR_CHECK(xrEnumerateSwapchainFormats(session, formatCount, &formatCount, formats.data()));
		bool formatFound = false;
		for (int64_t format : formats)
		{
			//if (format == static_cast<int64_t>(gState.io.format))
			if (format == static_cast<int64_t>(xr::colorFormat))
			{
				formatFound = true;
				break;
			}
		}
		VKA_ASSERT(formatFound);
	}
	// Create the swapchains and associated images
	swapchains.resize(eyeCount);
	eyeResolution = getEyeResolution(0u);
	// Create a swapchain and render targets
	for (uint32_t i = 0; i < eyeCount; i++)
	{
		const XrViewConfigurationView &eyeInfo = eyeImageInfos.at(0u);
		XrSwapchainCreateInfo          swapchainCreateInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
		swapchainCreateInfo.usageFlags  = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.format      = static_cast<int64_t>(xr::colorFormat);
		swapchainCreateInfo.sampleCount = VK_SAMPLE_COUNT_1_BIT;
		swapchainCreateInfo.width       = eyeInfo.recommendedImageRectWidth;
		swapchainCreateInfo.height      = eyeInfo.recommendedImageRectHeight;
		swapchainCreateInfo.faceCount   = 1u;
		swapchainCreateInfo.arraySize   = 1U;
		//swapchainCreateInfo.arraySize   = static_cast<uint32_t>(eyeCount);
		swapchainCreateInfo.mipCount    = 1u;
		VKXR_CHECK(xrCreateSwapchain(session, &swapchainCreateInfo, &swapchains[i]));

		// Get the number of swapchain images
	}
	uint32_t imageCount = 0u;
	uint32_t imageCountOutput = 0u;

	VKXR_CHECK(xrEnumerateSwapchainImages(swapchains[0], 0u, &imageCount, nullptr));
	VKA_ASSERT(imageCount = gState.io.imageCount);
	// Retrieve the swapchain images
	imagesLeft.resize(imageCount);
	imageViewsLeft.resize(imageCount);
	imageLayoutsLeft.resize(imageCount, VK_IMAGE_LAYOUT_UNDEFINED);
	{
		std::vector<XrSwapchainImageVulkan2KHR> swapchainImagesLeft(imageCount,
		                                                            {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN2_KHR});
		VKXR_CHECK(xrEnumerateSwapchainImages(swapchains[0], imageCount,
		                                      &imageCountOutput, reinterpret_cast<XrSwapchainImageBaseHeader *>(swapchainImagesLeft.data())));
		VKA_ASSERT(imageCount == imageCountOutput);
		for (uint32_t i = 0; i < imageCount; i++)
		{
			imagesLeft[i] = swapchainImagesLeft[i].image;
			ImageViewCreateInfo_Default viewCI = ImageViewCreateInfo_Default(imagesLeft[i], xr::colorFormat);
			VK_CHECK(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &imageViewsLeft[i]));
		}
	}
	imagesRight.resize(imageCount);
	imageViewsRight.resize(imageCount);
	imageLayoutsRight.resize(imageCount, VK_IMAGE_LAYOUT_UNDEFINED);
	{
		std::vector<XrSwapchainImageVulkan2KHR> swapchainImagesRight(imageCount,
		                                                             {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN2_KHR});
		VKXR_CHECK(xrEnumerateSwapchainImages(swapchains[1], imageCount,
		                                      &imageCountOutput, reinterpret_cast<XrSwapchainImageBaseHeader *>(swapchainImagesRight.data())));
		VKA_ASSERT(imageCount == imageCountOutput);
		for (uint32_t i = 0; i < imageCount; i++)
		{
			imagesRight[i] = swapchainImagesRight[i].image;
			ImageViewCreateInfo_Default viewCI = ImageViewCreateInfo_Default(imagesRight[i], xr::colorFormat);
			VK_CHECK(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &imageViewsRight[i]));
		}
	}

	// Create the eye render infos
	eyeRenderInfos.resize(eyeCount);
	for (uint32_t i = 0; i < eyeCount; i++)
	{
		XrCompositionLayerProjectionView &eyeRenderInfo = eyeRenderInfos[i];
		eyeRenderInfo.type                              = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
		eyeRenderInfo.next                              = nullptr;
		eyeRenderInfo.fov                               = eyePoses[i].fov;
		eyeRenderInfo.subImage.swapchain                = swapchains[i];
		eyeRenderInfo.subImage.imageArrayIndex          = 0;
		eyeRenderInfo.subImage.imageRect.offset.x       = 0;
		eyeRenderInfo.subImage.imageRect.offset.y       = 0;
		eyeRenderInfo.subImage.imageRect.extent.width   = eyeImageInfos[i].recommendedImageRectWidth;
		eyeRenderInfo.subImage.imageRect.extent.height  = eyeImageInfos[i].recommendedImageRectHeight;
	}

	eyeViewMatrices.resize(eyeCount);
	eyeProjectionMatrices.resize(eyeCount);
}

bool XrHeadset::beginSession() const
{
	XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
	sessionBeginInfo.primaryViewConfigurationType = xr::viewType;
	VKXR_CHECK(xrBeginSession(session, &sessionBeginInfo));
	return true;
}

bool XrHeadset::endSession() const
{
	VKXR_CHECK(xrEndSession(session));
	return true;
}

void XrHeadset::destroy()
{
	if (session)
	{
		xrEndSession(session);
	}

	for (XrSwapchain swapchain : swapchains)
	{
		xrDestroySwapchain(swapchain);
	}
	for (VkImageView imageView : imageViewsRight)
	{
		vkDestroyImageView(gState.device.logical, imageView, nullptr);
	}
	for (VkImageView imageView : imageViewsLeft)
	{
		vkDestroyImageView(gState.device.logical, imageView, nullptr);
	}
	imageViewsLeft.clear();
	imageViewsRight.clear();
	imagesLeft.clear();
	imagesRight.clear();
	imageLayoutsLeft.clear();
	imageLayoutsRight.clear();
	swapchains.clear();

	if (space)
	{
		xrDestroySpace(space);
		space = nullptr;
	}

	if (session)
	{
		xrDestroySession(session);
		session = nullptr;
	}

}


XrHeadset::BeginFrameResult XrHeadset::beginFrame(uint32_t &swapchainImageIndexLeft, uint32_t &swapchainImageIndexRight)
{
	const XrInstance instance = gState.xrContext.xrInstance;
	XrEventDataBuffer buffer{XR_TYPE_EVENT_DATA_BUFFER};
	while (xrPollEvent(instance, &buffer) == XR_SUCCESS)
	{
		switch (buffer.type)
		{
			case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
			{
				exitRequested = true;
				return BeginFrameResult::SkipFully;
			}
			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
				const XrEventDataSessionStateChanged &sessionStateChangedEvent =
				    *reinterpret_cast<XrEventDataSessionStateChanged *>(&buffer);
				sessionState = sessionStateChangedEvent.state;
				if (sessionState == XR_SESSION_STATE_READY)
				{
					beginSession();
				}
				else if (sessionState == XR_SESSION_STATE_STOPPING)
				{
					endSession();
				}
				else if (sessionState == XR_SESSION_STATE_EXITING || sessionState == XR_SESSION_STATE_LOSS_PENDING)
				{
					exitRequested = true;
					return BeginFrameResult::SkipFully;
				}
				break;
			}
		}
	}

	buffer.type = XR_TYPE_EVENT_DATA_BUFFER;
	if (sessionState != XR_SESSION_STATE_FOCUSED && sessionState != XR_SESSION_STATE_VISIBLE && sessionState != XR_SESSION_STATE_READY && sessionState != XR_SESSION_STATE_SYNCHRONIZED)
	{
		return BeginFrameResult::SkipRender;
	}

	XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO};
	VKXR_CHECK(xrWaitFrame(session, &frameWaitInfo, &frameState));
	XrFrameBeginInfo frameBeginInfo{XR_TYPE_FRAME_BEGIN_INFO};
	VKXR_CHECK(xrBeginFrame(session, &frameBeginInfo));
	// Update the eye poses
	viewState.type = XR_TYPE_VIEW_STATE;
	uint32_t         viewCount;
	XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
	viewLocateInfo.viewConfigurationType = xr::viewType;
	viewLocateInfo.displayTime            = frameState.predictedDisplayTime;
	viewLocateInfo.space                  = space;
	VKXR_CHECK(xrLocateViews(session, &viewLocateInfo, &viewState, static_cast<uint32_t>(eyePoses.size()), &viewCount, eyePoses.data()));

	// Update the eye render infos, view and projection matrices
	for (size_t i = 0; i < eyeCount; i++)
	{
		eyeRenderInfos[i].pose = eyePoses[i].pose;
		eyeRenderInfos[i].fov  = eyePoses[i].fov;
		eyeViewMatrices[i]     = glm::inverse(xr::poseToMatrix(eyePoses[i].pose));
		eyeProjectionMatrices[i] = xr::createProjectionMatrix(eyePoses[i].fov, 0.1f, 100.0f);
	}

	// Acquire the swapchain image
	XrSwapchainImageAcquireInfo swapchainImageAcquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
	VKXR_CHECK(xrAcquireSwapchainImage(swapchains[0], &swapchainImageAcquireInfo, &swapchainImageIndexLeft));
	VKXR_CHECK(xrAcquireSwapchainImage(swapchains[1], &swapchainImageAcquireInfo, &swapchainImageIndexRight));

	// Wait for the swapchain image to be available
	XrSwapchainImageWaitInfo swapchainImageWaitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
	swapchainImageWaitInfo.timeout = XR_INFINITE_DURATION;
	VKXR_CHECK(xrWaitSwapchainImage(swapchains[0], &swapchainImageWaitInfo));
	VKXR_CHECK(xrWaitSwapchainImage(swapchains[1], &swapchainImageWaitInfo));
	return BeginFrameResult::RenderFully;
}

void XrHeadset::endFrame() const
{
	// Release the swapchain image
	XrSwapchainImageReleaseInfo swapchainImageReleaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
	VKXR_CHECK(xrReleaseSwapchainImage(swapchains[0], &swapchainImageReleaseInfo));
	VKXR_CHECK(xrReleaseSwapchainImage(swapchains[1], &swapchainImageReleaseInfo));

	// End the frame
	XrCompositionLayerProjection layer{XR_TYPE_COMPOSITION_LAYER_PROJECTION};
	layer.space          = space;
	layer.viewCount     = static_cast<uint32_t>(eyeRenderInfos.size());
	layer.views         = eyeRenderInfos.data();

	std::vector<XrCompositionLayerBaseHeader *> layers;
	if (frameState.shouldRender)
	{
		layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader *>(&layer));
	}

	XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
	frameEndInfo.displayTime          = frameState.predictedDisplayTime;
	frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	//frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_ADDITIVE;
	frameEndInfo.layerCount           = layers.size();        // ?
	frameEndInfo.layers               = layers.data();
	VKXR_CHECK(xrEndFrame(session, &frameEndInfo));
}
}