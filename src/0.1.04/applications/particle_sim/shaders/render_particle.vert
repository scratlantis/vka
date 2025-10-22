#include "interface_structs.glsl"
layout(push_constant) uniform PC {PCRenderParticles pc;};
layout(location = 0) in vec3 pos;
out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
};

void main()
{
    gl_Position.xy = (pos.xy-pc.viewOffset)*pc.viewScale;
	gl_PointSize = pc.pointSize;
    // Map to Vulkan NDC
	gl_Position.zw = vec2(1.0);
    gl_Position.xy*=2.0;
    gl_Position.xy-=1.0;
}