#include "interface_structs.glsl"
layout(push_constant) uniform PC {PCRenderParticles pc;};
layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
