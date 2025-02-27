#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Camera.h"

namespace vka
{
struct FixedCameraState
{
	glm::vec3 fixpoint;
	glm::vec3 up;
	float     distance;
	float     yaw;
	float     pitch;
	float     moveSpeed;
	float     turnSpeed;
	float     scrollSpeed;
};

class FixedCamera : public Camera
{
  public:
	FixedCamera(FixedCameraState ci)
	{
		fixpoint      = ci.fixpoint;
		yaw           = ci.yaw;
		pitch         = ci.pitch;
		movementSpeed = ci.moveSpeed;
		turnSpeed     = ci.turnSpeed;
		scrollSpeed   = ci.scrollSpeed;
		distance      = ci.distance;
		worldUp       = ci.up;
		updateRotation();
	};
	FixedCameraState getState()
	{
		FixedCameraState state{};
		state.fixpoint    = fixpoint;
		state.yaw         = yaw;
		state.pitch       = pitch;
		state.moveSpeed   = movementSpeed;
		state.turnSpeed   = turnSpeed;
		state.scrollSpeed = scrollSpeed;
		state.up          = worldUp;
		state.distance    = distance;
		return state;
	}
	FixedCamera() = default;
	~FixedCamera(){};
	virtual bool      keyControl(float deltaTime) override;
	virtual bool      mouseControl(float deltaTime) override;
	virtual bool      keyControl() override;
	virtual bool      mouseControl() override;
	virtual glm::vec3 getPosition() const override;
	virtual glm::vec3 getViewDirection() const override;
	virtual glm::vec3 getViewUpDirection() const override;
	virtual glm::mat4 getViewMatrix() const override;

	glm::vec3 getFixpoint() const;
	void      setFixpoint(glm::vec3 newFixpoint);
  private:
	void      updateRotation();
	float     yaw;
	float     pitch;
	glm::vec3 worldUp;
	glm::vec3 fixpoint;
	float     movementSpeed;
	float     turnSpeed;
	float     scrollSpeed;
	float     distance;

	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
};
}        // namespace vka
