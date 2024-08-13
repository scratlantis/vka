#pragma once
#include <vka/vka.h>
struct ModelInfo
{
	std::string path;
	glm::vec3   offset;
	float       scale;
	float       yRotation;

	glm::mat4 getObjToWorldMatrix()
	{
		glm::mat4 objToWorld = glm::mat4(1.0f);
		objToWorld           = glm::translate(objToWorld, offset);
		objToWorld           = glm::rotate(objToWorld, glm::radians(yRotation), glm::vec3(0.0f, 1.0f, 0.0f));
		objToWorld           = glm::scale(objToWorld, glm::vec3(scale));
		return objToWorld;
	};
};