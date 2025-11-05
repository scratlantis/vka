#pragma once

#include <glm/fwd.hpp>

#include <openxr/openxr.h>

#include <vector>
#include <vka/core/core_common.h>

namespace vka
{
/*
 * The controllers class handles OpenXR controller support. It represents the controller system as a whole, not an
 * individual controller. This is more convenient due to the OpenXR API. It allows the application to retrieve the
 * current pose of a controller, which is then used to accurately pose the hand models in the scene. It also exposes the
 * current fly speed, which is used to fly the camera in the direction of the controller.
 */
class XrControllers final
{
  public:
	XrControllers() = default;
	~XrControllers() = default;

	void init();
	void destroy();
	bool sync(XrSpace space, XrTime time);


	glm::mat4 getPose(size_t controllerIndex) const;
	float     getFlySpeed(size_t controllerIndex) const;

  private:

	std::vector<XrPath>  paths;
	std::vector<XrSpace> spaces;

	std::vector<glm::mat4> poses;
	std::vector<float>     flySpeeds;

	XrActionSet actionSet  = nullptr;
	XrAction    poseAction = nullptr, flyAction = nullptr;

	DELETE_COPY_CONSTRUCTORS(XrControllers);
};
}        // namespace vka