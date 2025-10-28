#include "interface_structs.glsl"
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;

#ifdef ENABLE_DEBUG_COLOR
layout(location = 2) in vec3 inColorDebug;
#endif

struct PushStruct
{
	vec2	viewOffset;
	vec2	viewScale;
	vec2	extent;
	float	pointSize;
	float	intensity;
	float	velIntensity;
};
layout(push_constant) uniform PC {PushStruct pc;};

void main()
{
	outColor.a = 1.0;
	vec2 particleCenter = inPos * pc.extent;
	vec2 pixelCenter = gl_FragCoord.xy;
	float dist = length(pixelCenter - particleCenter);
	float normDist = dist / (0.5*pc.pointSize);
	float crossSection = clamp(1.0 - normDist*normDist, 0.0,1.0);
	outColor.rgb = inColor * crossSection * crossSection * crossSection*pc.intensity;
}
