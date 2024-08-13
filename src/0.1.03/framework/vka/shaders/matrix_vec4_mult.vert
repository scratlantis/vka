#version 460
#extension GL_GOOGLE_include_directive : enable
// Generates a fullscreen triangle. See https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/

layout(binding = 0) uniform VP{mat4 vp;};
layout(location = 0) in vec3 pos;
//layout(location = 0) out vec3 model_pos;


void main() 
{
	gl_Position = vp * vec4(pos, 1.0);
}