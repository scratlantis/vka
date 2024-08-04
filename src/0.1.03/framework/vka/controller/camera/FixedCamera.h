#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace vka
{
struct FixedCameraCI
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

struct FixedCameraCI_Default : public FixedCameraCI
{
	FixedCameraCI_Default()
	{
		fixpoint    = glm::vec3(0.0f, 0.0f, 0.0f);
		distance    = 1.0;
		up          = glm::vec3(0.0f, 1.0f, 0.0f);
		yaw         = 90.f;
		pitch       = 0.0f;
		moveSpeed   = 0.2f;
		turnSpeed   = 0.25f;
		scrollSpeed = 0.1f;
	}
};

class FixedCamera
{
  public:
	FixedCamera(FixedCameraCI ci)
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
	FixedCamera() : FixedCamera(FixedCameraCI_Default()){};
	~FixedCamera(){};
	void      keyControl(float deltaTime);
	void      mouseControl(float deltaTime);
	glm::vec3 getPosition() const;
	glm::vec3 getFixpoint() const;
	void      setFixpoint(glm::vec3 newFixpoint);
	glm::vec3 getViewDirection() const;
	glm::mat4 getViewMatrix() const;

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
