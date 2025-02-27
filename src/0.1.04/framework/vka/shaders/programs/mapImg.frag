
#include "../lib/format_conversion.glsl"
#include "../lib/tonemapping_operators.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PC
{
	float x;
	float y;
	float width;
	float height;
	float whitePoint;
	float exposure;
} pc;

layout(binding = 0) uniform sampler2D smp;

void main()
{
	vec4 color = texture(smp,inUV*vec2(pc.width, pc.height)+vec2(pc.x,pc.y));
#ifdef NORMALIZE
	if(color.a == 0.0)
	{
		color.xyz = vec3(0.0);
	}
	else
	{
		color.xyz = color.xyz / color.a;
	}
#endif
	color.a = 1.0;
#ifdef TONEMAPPING
	// https://bruop.github.io/tonemapping/
	vec3 rgb = color.rgb;
	// Yxy.x is Y, the luminance
	vec3 Yxy = convertRGB2Yxy(rgb);
	float lp = Yxy.x / (9.6 * pc.exposure + 0.0001);
	// Replace this line with other tone mapping functions
	// Here we applying the curve to the luminance exclusively
	Yxy.x = reinhard2(lp, pc.whitePoint);
	rgb = convertYxy2RGB(Yxy);
	color.rgb = toGamma( rgb );
#else

#ifdef GAMMA_CORRECTION
	color.rgb = toGamma( color.rgb );
#endif
#endif
	outColor = color;
}