#version 460
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PC
{
	float x;
	float y;
	float width;
	float height;
} pc;

layout(binding = 0) uniform sampler2D smp;

void main()
{
	vec4 color = texture(smp,inUV*vec2(pc.width, pc.height)+vec2(pc.x,pc.y));
	if(color.a == 0.0)
	{
		color.xyz = vec3(0.0);
	}
	else
	{
		color.xyz = color.xyz / color.a;
	}
	color.a = 1.0;
	outColor = color;
}