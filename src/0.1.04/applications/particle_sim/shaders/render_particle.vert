#include "interface_structs.glsl"
layout(push_constant) uniform PC {PCRenderParticles pc;};
layout(location = 0) in vec2 pos;
layout(location = 0) out vec2 outPos;
layout(location = 1) out vec3 outColor;


layout(binding = 0) readonly buffer PARTICLE_DENSITIES { float densities[];};

out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
};

void main()
{
    gl_Position.xy = (pos.xy-pc.viewOffset)*pc.viewScale;
	gl_PointSize = pc.pointSize;
    outPos = gl_Position.xy;
    outColor = densities[gl_VertexIndex] * vec3(1.0,0.0,0.0);
    // Map to Vulkan NDC
	gl_Position.zw = vec2(1.0);
    gl_Position.xy*=2.0;
    gl_Position.xy-=1.0;
}