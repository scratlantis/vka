#include "interface_structs.glsl"
#include "../lib/lib/type_vecn.glsl"

layout(location = 0) out vec4 outColor;
layout(location = 0) in vecN inPos;
layout(location = 1) in vec3 inColor;

struct PushStruct
{
	vec2	viewOffset;
	vec2	viewScale;
	vec2	extent;
	float	radius;
	float	intensity;
	float	velIntensity;
};
layout(push_constant) uniform PC {PushStruct pc;};

void main()
{
	outColor.a = 1.0;

	vec2 pixelCenter = gl_FragCoord.xy;
#if VECN_DIM == 2
	vec2 particleCenter = inPos * pc.extent;
	float dist = length(pixelCenter - particleCenter);
#elif VECN_DIM == 3
	Ray ray = genPrimaryRay(uvec2(pixelCenter));
	vec3 centerDir = normalize(inPos-ray.origin);
	float dist = sqrt(1-dot(centerDir, ray.direction)) * distance(ray.origin, inPos);
	dist*=1000.0;
#endif
	float normDist = dist / (0.5 * pc.radius);

	if(normDist > 1.0)
	{
		discard;
	}

	float crossSection = clamp(1.0 - normDist*normDist, 0.0,1.0);
	outColor.rgb = inColor * crossSection * crossSection * crossSection*pc.intensity;
}
