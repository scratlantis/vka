#version 460
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PC
{
	vec4 color;
} pc;

void main()
{
	outColor = pc.color;
}