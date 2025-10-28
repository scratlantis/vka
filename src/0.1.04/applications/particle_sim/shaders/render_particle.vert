#include "interface_structs.glsl"

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 vel;
layout(location = 0) out vec2 outPos;
layout(location = 1) out vec3 outColor;


layout(binding = 0) readonly buffer PARTICLE_DENSITIES { float densities[];};

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
	float	pointSize;
	float	intensity;
	float	velIntensity;
};
layout(push_constant) uniform PC {PushStruct pc;};


void main()
{
    gl_Position.xy = (pos.xy-pc.viewOffset)*pc.viewScale;
	gl_PointSize = pc.pointSize;
    outPos = gl_Position.xy;
    float density  = densities[gl_VertexIndex]; 
    outColor = vec3(abs(vel*pc.velIntensity),density);
    // Map to Vulkan NDC
	gl_Position.zw = vec2(1.0);
    gl_Position.xy*=2.0;
    gl_Position.xy-=1.0;
}