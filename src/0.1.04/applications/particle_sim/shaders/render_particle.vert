#include "interface_structs.glsl"
#include "../lib/lib/type_vecn.glsl"

layout(location = 0) in vecN pos;
layout(location = 1) in vecN vel;
layout(location = 0) out vecN outPos;
layout(location = 1) out vec3 outColor;


layout(binding = LOCAL_BINDING_OFFSET) readonly buffer PARTICLE_DENSITIES { float densities[];};
layout(binding = LOCAL_BINDING_OFFSET + 1) readonly buffer PARTICLE_FORCES { vecN forces[];};

out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
};

struct PushStruct
{
	vec2	viewOffset;
	vec2	viewScale;
	vec2	extent;
	float	radius;
	float	intensity;
	float	velIntensity;
	float	forceIntensity;
};
layout(push_constant) uniform PC {PushStruct pc;};


void main()
{
    float density  = densities[gl_VertexIndex]; 
    vecN force = forces[gl_VertexIndex];
#if VECN_DIM == 3
    outPos = pos;
    vec4 viewPos = worldToView(vec4(pos,1.0));
    gl_Position = viewToClip(viewPos);
    float dist = length(viewPos.xyz);
    gl_PointSize = pc.radius/max(dist,0.001);
    outColor = vec3(length(vel)*pc.velIntensity, length(force)*pc.forceIntensity*0.001, density);
    outColor.r = clamp(outColor.r - outColor.g, 0.0, 1.0);
#endif

#if VECN_DIM == 2
    gl_Position.xy = (pos.xy-pc.viewOffset)*pc.viewScale;
	gl_PointSize = pc.radius;
    outPos = gl_Position.xy;
    // Map to Vulkan NDC
	gl_Position.zw = vec2(1.0);
    gl_Position.xy*=2.0;
    gl_Position.xy-=1.0;
    outColor = vec3(abs(vel*pc.velIntensity),density);
#endif
}