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
	outColor = texture(smp,inUV*vec2(pc.width, pc.height)+vec2(pc.x,pc.y));
}