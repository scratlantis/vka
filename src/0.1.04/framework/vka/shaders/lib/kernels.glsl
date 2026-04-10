#ifndef KERNELS_H
#define KERNELS_H

#include "math_const.glsl"

// Notes:
// - Prefer the _r2 variants when you already have squared distance.
// - The smooth derivative was fixed: it needs (rad^2 - dst^2)^2.
// - No pow() in hot functions.
// - Normalization functions are still here, but for best performance you can
//   precompute them once on CPU / as uniforms if radius is constant per dispatch.

#define KERNEL_ID_QUADRATIC 0
#define KERNEL_ID_SMOOTH    1

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

float sqr(float x)
{
    return x * x;
}

float pow4(float x)
{
    float x2 = x * x;
    return x2 * x2;
}

float pow5(float x)
{
    float x2 = x * x;
    float x4 = x2 * x2;
    return x4 * x;
}

float pow8(float x)
{
    float x2 = x * x;
    float x4 = x2 * x2;
    return x4 * x4;
}

float pow9(float x)
{
    float x2 = x * x;
    float x4 = x2 * x2;
    float x8 = x4 * x4;
    return x8 * x;
}

// ------------------------------------------------------------
// Quadratic kernel
// W(r) ~ max(0, h - r)^2
// ------------------------------------------------------------

float kernelWeight_quadratic(float dst, float rad)
{
    float t = max(0.0, rad - dst);
    return t * t;
}

// Useful when you already have dst^2.
// This still needs sqrt because the kernel depends on (h - r), not just r^2.
float kernelWeight_quadratic_r2(float dst2, float rad)
{
    float rad2 = rad * rad;
    if (dst2 >= rad2)
    {
        return 0.0;
    }

    float dst = sqrt(dst2);
    float t = rad - dst;
    return t * t;
}

float kernelNormalization_quadratic(float rad)
{
#if VECN_DIM == 3
    return 15.0 / (PI * pow5(rad));
#else
    return 6.0 / (PI * pow4(rad));
#endif
}

// Radial derivative dW/dr
float kernelDerivative_quadratic(float dst, float rad)
{
    if (dst >= rad)
    {
        return 0.0;
    }

#if VECN_DIM == 3
    float scale = 30.0 / (PI * pow5(rad));
#else
    float scale = 12.0 / (PI * pow4(rad));
#endif

    return scale * (dst - rad);
}

// ------------------------------------------------------------
// Smooth kernel
// W(r) ~ max(0, h^2 - r^2)^3
// ------------------------------------------------------------

float kernelWeight_smooth(float dst, float rad)
{
    float rad2 = rad * rad;
    float dst2 = dst * dst;
    float t = max(0.0, rad2 - dst2);
    return t * t * t;
}

// Preferred version for density pass if you already compute squared distance.
float kernelWeight_smooth_r2(float dst2, float rad)
{
    float rad2 = rad * rad;
    float t = max(0.0, rad2 - dst2);
    return t * t * t;
}

float kernelNormalization_smooth(float rad)
{
#if VECN_DIM == 3
    return 315.0 / (64.0 * PI * pow9(rad));
#else
    return 4.0 / (PI * pow8(rad));
#endif
}

// Radial derivative dW/dr
// For W(r) = C * (h^2 - r^2)^3,
// dW/dr = C * (-6r) * (h^2 - r^2)^2
float kernelDerivative_smooth(float dst, float rad)
{
    if (dst >= rad)
    {
        return 0.0;
    }

    float rad2 = rad * rad;
    float dst2 = dst * dst;
    float t = rad2 - dst2;

#if VECN_DIM == 3
    float scale = -945.0 / (32.0 * PI * pow9(rad));
#else
    float scale = -24.0 / (PI * pow8(rad));
#endif

    return scale * dst * t * t;
}

// ------------------------------------------------------------
// Selection macros
// ------------------------------------------------------------

#ifdef SELECT_KERNEL_TYPE

    #if SELECT_KERNEL_TYPE == KERNEL_ID_QUADRATIC

        #define KERNEL_WEIGHT(dst, rad)            kernelWeight_quadratic(dst, rad)
        #define KERNEL_WEIGHT_R2(dst2, rad)        kernelWeight_quadratic_r2(dst2, rad)
        #define KERNEL_NORMALIZATION(rad)          kernelNormalization_quadratic(rad)
        #define KERNEL_DERIVATIVE(dst, rad)        kernelDerivative_quadratic(dst, rad)

    #elif SELECT_KERNEL_TYPE == KERNEL_ID_SMOOTH

        #define KERNEL_WEIGHT(dst, rad)            kernelWeight_smooth(dst, rad)
        #define KERNEL_WEIGHT_R2(dst2, rad)        kernelWeight_smooth_r2(dst2, rad)
        #define KERNEL_NORMALIZATION(rad)          kernelNormalization_smooth(rad)
        #define KERNEL_DERIVATIVE(dst, rad)        kernelDerivative_smooth(dst, rad)

    #endif

#endif

#endif
//#ifndef KERNELS_H
//#define KERNELS_H
//#include "math_const.glsl"
//
//#define KERNEL_ID_QUADRATIC 0
//float kernelWeight_quadratic(float dst, float rad)
//{
//	float weight = max(0, rad - dst);
//	return weight * weight;
//}
//float kernelNormalization_quadratic(float rad)
//{
//#if VECN_DIM == 3
//	return 15.0/(PI * pow(rad,5));
//#else
//	return 6.0/(PI * pow(rad,4));
//#endif
//}
//float kernelDerivative_quadratic(float dst, float rad)
//{
//	if(dst >= rad)
//	{
//		return 0.0;
//	}
//	float scale = 12/(PI * pow(rad,4));
//	return scale * (dst - rad);
//}
//
//#define KERNEL_ID_SMOOTH 1
//float kernelWeight_smooth(float dst, float rad)
//{
//	float weight = max(0, rad*rad - dst*dst);
//	return weight * weight * weight;
//}
//float kernelNormalization_smooth(float rad)
//{
//#if VECN_DIM == 3
//	return 315.0/(64.0 * PI * pow(rad,9));
//#else
//	return 4.0/(PI * pow(rad,8));
//#endif
//}
//float kernelDerivative_smooth(float dst, float rad)
//{
//	if(dst >= rad)
//	{
//		return 0.0;
//	}
//	float scale = -24/(PI * pow(rad,8));
//	return scale * dst * (dst * dst - rad * rad);
//}
//
//
//
//#ifdef SELECT_KERNEL_TYPE
//	#if SELECT_KERNEL_TYPE == KERNEL_ID_QUADRATIC
//		#define KERNEL_WEIGHT(dst, rad) kernelWeight_quadratic(dst, rad)
//		#define KERNEL_NORMALIZATION(rad) kernelNormalization_quadratic(rad)
//		#define KERNEL_DERIVATIVE(dst, rad) kernelDerivative_quadratic(dst, rad)
//	#elif SELECT_KERNEL_TYPE == KERNEL_ID_SMOOTH
//		#define KERNEL_WEIGHT(dst, rad) kernelWeight_smooth(dst, rad)
//		#define KERNEL_NORMALIZATION(rad) kernelNormalization_smooth(rad)
//		#define KERNEL_DERIVATIVE(dst, rad) kernelDerivative_smooth(dst, rad)
//	#endif
//#endif
//
//#endif