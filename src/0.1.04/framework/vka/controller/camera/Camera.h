#pragma once
#include <vka/core/core_common.h>
namespace vka
{
	class Camera
	{
	  public:
		Camera(){};
		~Camera(){};
	    virtual bool      keyControl(float deltaTime)   = 0;
	    virtual bool      mouseControl(float deltaTime) = 0;
	    virtual bool      keyControl()                  = 0;
	    virtual bool      mouseControl()                = 0;
	    virtual glm::vec3 getPosition() const           = 0;
	    virtual glm::vec3 getViewDirection() const      = 0;
	    virtual glm::vec3 getViewUpDirection() const    = 0;
		virtual glm::mat4 getViewMatrix() const         = 0;
	};
}
