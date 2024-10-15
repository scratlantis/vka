layout(binding = 0) uniform VP{mat4 vp;};
layout(location = 0) in vec3 pos;


void main() 
{
	gl_Position = vp * vec4(pos, 1.0);
}