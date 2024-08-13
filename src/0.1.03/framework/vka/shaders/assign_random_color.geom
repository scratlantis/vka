#version 460
#extension GL_GOOGLE_include_directive : enable
#include "random.glsl"

layout(triangles) in;
//layout(location = 0) in vec3 model_pos[];

layout(triangle_strip, max_vertices = 3) out;
layout(location = 0) out vec3 color;
void main()
{
	vec3 randomColor = random3D(gl_PrimitiveIDIn);
	color = randomColor;
	gl_Position = gl_in[0].gl_Position;
	color = randomColor;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	color = randomColor;
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	color = randomColor;
	EmitVertex();
}