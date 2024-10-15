#include "misc.h"
#include <glm/ext/matrix_transform.hpp>
namespace vka
{

glm::mat4 getMatrix(glm::vec3 pos, glm::vec3 rotDeg, float scale)
{
	glm::mat4 objToWorld = glm::mat4(1.0f);
	objToWorld           = glm::translate(objToWorld, pos);
	objToWorld           = glm::rotate(objToWorld, glm::radians(rotDeg.x), glm::vec3(1.0f, 0.0f, 0.0f));
	objToWorld           = glm::rotate(objToWorld, glm::radians(rotDeg.y), glm::vec3(0.0f, 1.0f, 0.0f));
	objToWorld           = glm::rotate(objToWorld, glm::radians(rotDeg.z), glm::vec3(0.0f, 0.0f, 1.0f));
	objToWorld           = glm::scale(objToWorld, glm::vec3(scale));
	return objToWorld;
};
}