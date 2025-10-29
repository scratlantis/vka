#ifndef KERNELS_H
#define KERNELS_H
#include "math_const.glsl"

#define KERNEL_ID_QUADRATIC 0
float kernelWeight_quadratic(float dst, float rad)
{
	float weight = max(0, rad - dst);
	return weight * weight;
}
float kernelNormalization_quadratic(float rad)
{
#if VECN_DIM == 3
	return 15.0/(PI * pow(rad,5));
#else
	return 6.0/(PI * pow(rad,4));
#endif
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

#define KERNEL_ID_SMOOTH 1
float kernelWeight_smooth(float dst, float rad)
{
	float weight = max(0, rad*rad - dst*dst);
	return weight * weight * weight;
}
float kernelNormalization_smooth(float rad)
{
#if VECN_DIM == 3
	return 315.0/(64.0 * PI * pow(rad,9));
#else
	return 4.0/(PI * pow(rad,8));
#endif
}
float kernelDerivative_smooth(float dst, float rad)
{
	if(dst >= rad)
	{
		return 0.0;
	}
	float scale = -24/(PI * pow(rad,8));
	return scale * dst * (dst * dst - rad * rad);
}



#ifdef SELECT_KERNEL_TYPE
	#if SELECT_KERNEL_TYPE == KERNEL_ID_QUADRATIC
		#define KERNEL_WEIGHT(dst, rad) kernelWeight_quadratic(dst, rad)
		#define KERNEL_NORMALIZATION(rad) kernelNormalization_quadratic(rad)
		#define KERNEL_DERIVATIVE(dst, rad) kernelDerivative_quadratic(dst, rad)
	#elif SELECT_KERNEL_TYPE == KERNEL_ID_SMOOTH
		#define KERNEL_WEIGHT(dst, rad) kernelWeight_smooth(dst, rad)
		#define KERNEL_NORMALIZATION(rad) kernelNormalization_smooth(rad)
		#define KERNEL_DERIVATIVE(dst, rad) kernelDerivative_smooth(dst, rad)
	#endif
#endif

#endif