#include "diff_operations.glsl"

#ifndef DIFF_OP
#define DIFF_OP DIFF_OP_COMPONENT_SUM
#endif

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PC
{
	float x;
	float y;
	float width;
	float height;
} pc;

layout(binding = 0) uniform sampler2D smpA;
layout(binding = 1) uniform sampler2D smpB;



void main()
{
	vec4 colorA = texture(smpA,inUV*vec2(pc.width, pc.height)+vec2(pc.x,pc.y));
	vec4 colorB = texture(smpB,inUV*vec2(pc.width, pc.height)+vec2(pc.x,pc.y));


	if(colorA.a == 0.0)
	{
		colorA.xyz = vec3(0.0);
	}
	else
	{
		colorA.xyz = colorA.xyz / colorA.a;
	}
	
	if(colorB.a == 0.0)
	{
		colorB.xyz = vec3(0.0);
	}
	else
	{
		colorB.xyz = colorB.xyz / colorB.a;
	}



	outColor.a = 1.0;
	#if(DIFF_OP == DIFF_OP_COMPONENT_SUM_DIFF)
	outColor.rgb = vec3(0.5 + -(colorA.r - colorB.r + colorA.g - colorB.g + colorA.b - colorB.b));
	outColor.rgb = vec3(distance(colorA, colorB))*1.0;
	//outColor.rgb = colorA.rgb;
	#elif(DIFF_OP == DIFF_OP_PER_COMPONENT_SQUARED_DIFF_SUM)
	vec3 diff = colorA.rgb - colorB.rgb;
	diff = diff * diff;
	outColor.rgb = vec3(diff.x + diff.y + diff.z);
	#endif
	//outColor.rgb = vec3(colorB.r);
}