#include "XrControllers.h"
#include <glm/mat4x4.hpp>

#include <vka/globals.h>
#include "xr_common.h"

#include <array>
#include <cstring>

namespace vka
{
constexpr size_t controllerCount = 2u;

const std::string actionSetName          = "actionset";
const std::string localizedActionSetName = "Actions";

void XrControllers::init()
{
	XrInstance instance = gState.xrContext.xrInstance;
	XrSession  session  = gState.xrHeadset.session;
	// Create an action set
	XrActionSetCreateInfo actionSetCreateInfo{XR_TYPE_ACTION_SET_CREATE_INFO};

	memcpy(actionSetCreateInfo.actionSetName, actionSetName.data(), actionSetName.length() + 1u);
	memcpy(actionSetCreateInfo.localizedActionSetName, localizedActionSetName.data(),
	       localizedActionSetName.length() + 1u);

	VKXR_CHECK(xrCreateActionSet(instance, &actionSetCreateInfo, &actionSet));
	// Create paths
	paths.resize(controllerCount);
	paths.at(0u) = xr::stringToPath(instance, "/user/hand/left");
	paths.at(1u) = xr::stringToPath(instance, "/user/hand/right");

	// Create actions
	VKA_ASSERT(xr::createAction(actionSet, paths, "handpose", "Hand Pose", XR_ACTION_TYPE_POSE_INPUT, poseAction));
	VKA_ASSERT(xr::createAction(actionSet, paths, "grab", "Grab", XR_ACTION_TYPE_FLOAT_INPUT, grabAction));
	VKA_ASSERT(xr::createAction(actionSet, paths, "apress", "A Press", XR_ACTION_TYPE_BOOLEAN_INPUT, aPressAction));
	VKA_ASSERT(xr::createAction(actionSet, paths, "bpress", "B Press", XR_ACTION_TYPE_BOOLEAN_INPUT, bPressAction));
	VKA_ASSERT(xr::createAction(actionSet, paths, "trigger", "Trigger", XR_ACTION_TYPE_FLOAT_INPUT, triggerAction));
	VKA_ASSERT(xr::createAction(actionSet, paths, "thumbstickclick", "Thumbstick Click", XR_ACTION_TYPE_BOOLEAN_INPUT, thumbstickClickAction));

	// Create spaces
	spaces.resize(controllerCount);
	for (size_t controllerIndex = 0u; controllerIndex < controllerCount; ++controllerIndex)
	{
		const XrPath &path = paths.at(controllerIndex);

		XrActionSpaceCreateInfo actionSpaceCreateInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
		actionSpaceCreateInfo.action            = poseAction;
		actionSpaceCreateInfo.poseInActionSpace = xr::makeIdentity();
		actionSpaceCreateInfo.subactionPath     = path;

		VKXR_CHECK(xrCreateActionSpace(session, &actionSpaceCreateInfo, &spaces.at(controllerIndex)));
	}

	const std::array<XrActionSuggestedBinding, 12u> indexBindings = {
	    {{grabAction, xr::stringToPath(instance, "/user/hand/left/input/squeeze/value")},
	     {grabAction, xr::stringToPath(instance, "/user/hand/right/input/squeeze/value")},
	     {aPressAction, xr::stringToPath(instance, "/user/hand/left/input/a/click")},
	     {aPressAction, xr::stringToPath(instance, "/user/hand/right/input/a/click")},
	     {bPressAction, xr::stringToPath(instance, "/user/hand/left/input/b/click")},
	     {bPressAction, xr::stringToPath(instance, "/user/hand/right/input/b/click")},
	     {triggerAction, xr::stringToPath(instance, "/user/hand/left/input/trigger/value")},
	     {triggerAction, xr::stringToPath(instance, "/user/hand/right/input/trigger/value")},
	     {poseAction, xr::stringToPath(instance, "/user/hand/left/input/aim/pose")},
	     {poseAction, xr::stringToPath(instance, "/user/hand/right/input/aim/pose")},
		 {thumbstickClickAction, xr::stringToPath(instance, "/user/hand/left/input/thumbstick/click")},
		{thumbstickClickAction, xr::stringToPath(instance, "/user/hand/right/input/thumbstick/click")}
		}};
	
	XrInteractionProfileSuggestedBinding interactionProfileSuggestedBinding = {XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
	interactionProfileSuggestedBinding.interactionProfile =
	    xr::stringToPath(instance, "/interaction_profiles/valve/index_controller");
	interactionProfileSuggestedBinding.suggestedBindings      = indexBindings.data();
	interactionProfileSuggestedBinding.countSuggestedBindings = static_cast<uint32_t>(indexBindings.size());
	VKXR_CHECK(xrSuggestInteractionProfileBindings(instance, &interactionProfileSuggestedBinding));


	// Attach the controller action set
	XrSessionActionSetsAttachInfo sessionActionSetsAttachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
	sessionActionSetsAttachInfo.countActionSets = 1u;
	sessionActionSetsAttachInfo.actionSets      = &actionSet;

	VKXR_CHECK(xrAttachSessionActionSets(session, &sessionActionSetsAttachInfo));

	flySpeeds.resize(controllerCount);
	poses.resize(controllerCount);
	grapStrength.resize(controllerCount);
	aPress.resize(controllerCount);
	bPress.resize(controllerCount);
	triggerPressure.resize(controllerCount);
	thumbstickClick.resize(controllerCount);
}

void XrControllers::destroy()
{
	for (const XrSpace &space : spaces)
	{
		xrDestroySpace(space);
	}
	if (flyAction)
	{
		xrDestroyAction(flyAction);
	}

	if (poseAction)
	{
		xrDestroyAction(poseAction);
	}

	if (actionSet)
	{
		xrDestroyActionSet(actionSet);
	}
}

bool XrControllers::sync(XrSpace space, XrTime time)
{
	aPressLastFrame = aPress;
	bPressLastFrame = bPress;
	thumbstickClickLastFrame = thumbstickClick;


	// Sync the actions
	XrActiveActionSet activeActionSet;
	activeActionSet.actionSet     = actionSet;
	activeActionSet.subactionPath = XR_NULL_PATH;        // Wildcard for all

	XrActionsSyncInfo actionsSyncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
	actionsSyncInfo.countActiveActionSets = 1u;
	actionsSyncInfo.activeActionSets      = &activeActionSet;

	XrSession session = gState.xrHeadset.session;

	VKXR_CHECK(xrSyncActions(gState.xrHeadset.session, &actionsSyncInfo));

	// Update the actions
	for (size_t controllerIndex = 0u; controllerIndex < controllerCount; ++controllerIndex)
	{
		const XrPath &path = paths.at(controllerIndex);

		// Pose
		XrActionStatePose poseState{XR_TYPE_ACTION_STATE_POSE};
		VKXR_CHECK(xr::updateActionStatePose(session, poseAction, path, poseState));

		if (poseState.isActive)
		{
			XrSpaceLocation spaceLocation{XR_TYPE_SPACE_LOCATION};
			VKXR_CHECK(xrLocateSpace(spaces.at(controllerIndex), space, time, &spaceLocation));

			// Check that the position and orientation are valid and tracked
			constexpr XrSpaceLocationFlags checkFlags =
			    XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT |
			    XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
			if ((spaceLocation.locationFlags & checkFlags) == checkFlags)
			{
				poses.at(controllerIndex) = xr::poseToMatrix(spaceLocation.pose);
			}
		}
		{
			// Grab strength
			XrActionStateFloat grabStrengthState{XR_TYPE_ACTION_STATE_FLOAT};
			VKXR_CHECK(xr::updateActionStateFloat(session, grabAction, path, grabStrengthState));
			if (grabStrengthState.isActive)
			{
				grapStrength.at(controllerIndex) = grabStrengthState.currentState;
			}

			// A button press
			XrActionStateBoolean aPressState{XR_TYPE_ACTION_STATE_BOOLEAN};
			VKXR_CHECK(xr::updateActionStateBoolean(session, aPressAction, path, aPressState));
			if (aPressState.isActive)
			{
				aPress.at(controllerIndex) = static_cast<bool>(aPressState.currentState);
			}

			// B button press
			XrActionStateBoolean bPressState{XR_TYPE_ACTION_STATE_BOOLEAN};
			VKXR_CHECK(xr::updateActionStateBoolean(session, bPressAction, path, bPressState));
			if (bPressState.isActive)
			{
				bPress.at(controllerIndex) = static_cast<bool>(bPressState.currentState);
			}

			// Trigger pressure
			XrActionStateFloat triggerPressureState{XR_TYPE_ACTION_STATE_FLOAT};
			VKXR_CHECK(xr::updateActionStateFloat(session, triggerAction, path, triggerPressureState));
			if (triggerPressureState.isActive)
			{
				triggerPressure.at(controllerIndex) = triggerPressureState.currentState;
			}

			XrActionStateBoolean thumbstickClickState{XR_TYPE_ACTION_STATE_BOOLEAN};
			VKXR_CHECK(xr::updateActionStateBoolean(session, thumbstickClickAction, path, thumbstickClickState));
			if (thumbstickClickState.isActive)
			{
				thumbstickClick.at(controllerIndex) = static_cast<bool>(thumbstickClickState.currentState);
			}

		}
	}
	return true;
}

glm::mat4 XrControllers::getPose(size_t controllerIndex) const
{
	return poses.at(controllerIndex);
}

float XrControllers::getFlySpeed(size_t controllerIndex) const
{
	return flySpeeds.at(controllerIndex);
}

float XrControllers::getGrabStrength(size_t controllerIndex) const
{
	return grapStrength.at(controllerIndex);
}

bool XrControllers::getAPressed(size_t controllerIndex) const
{
	return aPress.at(controllerIndex);
}

bool XrControllers::getBPressed(size_t controllerIndex) const
{
	return bPress.at(controllerIndex);
}

float XrControllers::getTriggerPressure(size_t controllerIndex) const
{
	return triggerPressure.at(controllerIndex);
}

bool XrControllers::getThumbstickPressed(size_t controllerIndex) const
{
	return thumbstickClick.at(controllerIndex);
}


bool XrControllers::getAPress(size_t controllerIndex) const
{
	return aPress.at(controllerIndex) && !aPressLastFrame.at(controllerIndex);
}

bool XrControllers::getBPress(size_t controllerIndex) const
{
	return bPress.at(controllerIndex) && !bPressLastFrame.at(controllerIndex);
}

bool XrControllers::getThumbstickPress(size_t controllerIndex) const
{
	return thumbstickClick.at(controllerIndex) && !thumbstickClickLastFrame.at(controllerIndex);
}









}        // namespace