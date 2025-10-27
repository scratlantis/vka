#include "interface_structs.glsl"
layout(push_constant) uniform PC {PCRenderParticles pc;};
layout(location = 0) in vec2 pos;
layout(location = 0) out vec2 outPos;
layout(location = 1) out vec3 outColor;


layout(binding = 0) readonly buffer PARTICLE_DENSITIES { float densities[];};

#ifdef ENABLE_DEBUG_COLOR
layout(binding = 1) readonly buffer DEBUG_COLORS { vec4 debug_colors[];};
layout(location = 2) out vec3 outColorDebug;
#endif

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
    outColor = vec3(densities[gl_VertexIndex],0.0,0.0);
    // Map to Vulkan NDC
	gl_Position.zw = vec2(1.0);
    gl_Position.xy*=2.0;
    gl_Position.xy-=1.0;

    #ifdef ENABLE_DEBUG_COLOR
    outColorDebug = debug_colors[gl_VertexIndex].rgb;
    #endif
}