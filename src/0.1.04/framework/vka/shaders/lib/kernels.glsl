#ifndef KERNELS_H
#define KERNELS_H
#include "math_const.glsl"

float kernelWeight_quadratic(float dst, float rad)
{
	float weight = max(0, rad - dst);
	return weight * weight;
}
float kernelNormalization_quadratic(float rad)
{
	return 6.0/(PI * pow(rad,4));
}
float kernelDerivative_quadratic(float dst, float rad)
{
	if(dst >= rad)
	{
		return 0.0;
	}
	float scale = 12/(PI * pow(rad,4));
	return scale * (dst - rad);
}

#define KERNEL_ID_QUADRATIC 0

#ifdef SELECT_KERNEL_TYPE
	#if SELECT_KERNEL_TYPE == KERNEL_ID_QUADRATIC
		#define KERNEL_WEIGHT(dst, rad) kernelWeight_quadratic(dst, rad)
		#define KERNEL_NORMALIZATION(rad) kernelNormalization_quadratic(rad)
		#define KERNEL_DERIVATIVE(dst, rad) kernelDerivative_quadratic(dst, rad)
	#endif
#endif

#endif