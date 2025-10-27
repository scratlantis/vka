#include "interface_structs.glsl"
layout(push_constant) uniform PC {PCRenderParticles pc;};
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;

#ifdef ENABLE_DEBUG_COLOR
layout(location = 2) in vec3 inColorDebug;
#endif


void main()
{
	outColor.a = 1.0;
	vec2 particleCenter = inPos * pc.extent;
	vec2 pixelCenter = gl_FragCoord.xy;
	float dist = length(pixelCenter - particleCenter);
	float normDist = dist / (0.5*pc.pointSize);
	float crossSection = clamp(1.0 - normDist*normDist, 0.0,1.0);
	outColor.rgb = inColor * crossSection * crossSection * crossSection*pc.intensity;
	if(dist < 1.0)
	{
	//#ifdef ENABLE_DEBUG_COLOR
	//	outColor.rgb = inColorDebug;
	//#else
	//	outColor.rgb = vec3(0.0,1.0,0.0);
	//#endif
	outColor.rgb = vec3(0.0,1.0,0.0);
	}
}
