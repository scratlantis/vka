#include "FixedCamera.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vka/globals.h>
namespace vka
{
bool FixedCamera::keyControl(float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
	bool *keys     = gState.io.keyPressed;
	bool hasChanged = false;
	if (keys[GLFW_KEY_W])
	{
		fixpoint += front * velocity;
		hasChanged = true;
	}

	if (keys[GLFW_KEY_D])
	{
		fixpoint += right * velocity;
		hasChanged = true;
	}

	if (keys[GLFW_KEY_A])
	{
		fixpoint += -right * velocity;
		hasChanged = true;
	}

	if (keys[GLFW_KEY_S])
	{
		fixpoint += -front * velocity;
		hasChanged = true;
	}

	if (keys[GLFW_KEY_SPACE])
	{
		fixpoint += -worldUp * velocity;
		hasChanged = true;
	}

	if (keys[GLFW_KEY_LEFT_SHIFT])
	{
		fixpoint += worldUp * velocity;
		hasChanged = true;
	}

	if (keys[GLFW_KEY_P])
	{
		std::cout << "Camera Fixpoint: " << glm::to_string(fixpoint) << std::endl;
	}

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	float deltaDistance = scrollSpeed * gState.io.mouse.scrollChange * deltaTime / 0.016;
	distance -= deltaDistance;
	
	hasChanged = hasChanged || deltaDistance != 0;

	if (distance < 0.1)
	{
		distance = 0.1;
	}
	return hasChanged;
}

bool FixedCamera::mouseControl(float deltaTime)
{
	float xChange = turnSpeed * gState.io.mouse.change.x * deltaTime / 0.016;
	float yChange = turnSpeed * gState.io.mouse.change.y * deltaTime / 0.016;

	yaw += xChange;
	pitch += yChange;

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}
	updateRotation();

	return xChange != 0 && yChange != 0;
}

glm::vec3 FixedCamera::getPosition() const
{
	return fixpoint - front * distance;
}

glm::vec3 FixedCamera::getFixpoint() const
{
	return fixpoint;
}

void FixedCamera::setFixpoint(glm::vec3 newFixpoint)
{
	fixpoint = newFixpoint;
}

glm::vec3 FixedCamera::getViewDirection() const
{
	return front;
}

glm::mat4 FixedCamera::getViewMatrix() const
{
	return glm::lookAt(getPosition(), getPosition() + front, up);
}

void FixedCamera::updateRotation()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front   = glm::normalize(front);
	right   = glm::normalize(glm::cross(front, worldUp));
	up      = glm::normalize(glm::cross(right, front));
}
}        // namespace vka
