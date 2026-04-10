#ifndef KERNELS_H
#define KERNELS_H

#include "math_const.glsl"

#define KERNEL_ID_QUADRATIC 0
#define KERNEL_ID_SMOOTH    1

float sqr(float x) { return x * x; }

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

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
// Quadratic
// W(r) ~ (h - r)^2
// ------------------------------------------------------------

float kernelShape_quadratic(float dst, float rad)
{
    float t = max(0.0, rad - dst);
    return t * t;
}

float kernelShape_quadratic_r2(float dst2, float rad, float rad2)
{
    if (dst2 >= rad2) return 0.0;
    float dst = sqrt(dst2);
    float t = rad - dst;
    return t * t;
}

// Returns only the radial shape part, without normalization:
// d/dr [(h-r)^2] = 2(r-h)
// We fold the factor 2 * normalization into kernelDerivativeScale().
float kernelDerivShape_quadratic(float dst, float rad)
{
    if (dst >= rad) return 0.0;
    return (dst - rad);
}

// 2D:  6 / (pi * r^4)
// 3D: 15 / (pi * r^5)
float kernelDensityNorm_quadratic(float rad)
{
#if VECN_DIM == 3
    return 15.0 / (PI * pow5(rad));
#else
    return 6.0 / (PI * pow4(rad));
#endif
}

// 2D: 12 / (pi * r^4)
// 3D: 30 / (pi * r^5)
float kernelDerivativeScale_quadratic(float rad)
{
#if VECN_DIM == 3
    return 30.0 / (PI * pow5(rad));
#else
    return 12.0 / (PI * pow4(rad));
#endif
}

// ------------------------------------------------------------
// Smooth
// W(r) ~ (h^2 - r^2)^3
// ------------------------------------------------------------

float kernelShape_smooth(float dst, float rad)
{
    float rad2 = rad * rad;
    float dst2 = dst * dst;
    float t = max(0.0, rad2 - dst2);
    return t * t * t;
}

float kernelShape_smooth_r2(float dst2, float rad2)
{
    float t = max(0.0, rad2 - dst2);
    return t * t * t;
}

// Returns only the radial shape part, without normalization:
// d/dr [(h^2-r^2)^3] = -6r(h^2-r^2)^2
// We fold -6 * normalization into kernelDerivativeScale().
float kernelDerivShape_smooth(float dst, float rad, float rad2)
{
    if (dst >= rad) return 0.0;
    float t = rad2 - dst * dst;
    return dst * t * t;
}

// 2D:         4 / (pi * r^8)
// 3D: 315 / (64 pi * r^9)
float kernelDensityNorm_smooth(float rad)
{
#if VECN_DIM == 3
    return 315.0 / (64.0 * PI * pow9(rad));
#else
    return 4.0 / (PI * pow8(rad));
#endif
}

// 2D:       -24 / (pi * r^8)
// 3D: -945 / (32 pi * r^9)
float kernelDerivativeScale_smooth(float rad)
{
#if VECN_DIM == 3
    return -945.0 / (32.0 * PI * pow9(rad));
#else
    return -24.0 / (PI * pow8(rad));
#endif
}

// ------------------------------------------------------------
// Selection
// ------------------------------------------------------------

#ifdef SELECT_KERNEL_TYPE
    #if SELECT_KERNEL_TYPE == KERNEL_ID_QUADRATIC
        #define KERNEL_SHAPE(dst, rad)                 kernelShape_quadratic(dst, rad)
        #define KERNEL_SHAPE_R2(dst2, rad, rad2)      kernelShape_quadratic_r2(dst2, rad, rad2)
        #define KERNEL_DERIV_SHAPE(dst, rad, rad2)    kernelDerivShape_quadratic(dst, rad)
        #define KERNEL_DENSITY_NORM(rad)              kernelDensityNorm_quadratic(rad)
        #define KERNEL_DERIV_SCALE(rad)               kernelDerivativeScale_quadratic(rad)
    #elif SELECT_KERNEL_TYPE == KERNEL_ID_SMOOTH
        #define KERNEL_SHAPE(dst, rad)                 kernelShape_smooth(dst, rad)
        #define KERNEL_SHAPE_R2(dst2, rad, rad2)      kernelShape_smooth_r2(dst2, rad2)
        #define KERNEL_DERIV_SHAPE(dst, rad, rad2)    kernelDerivShape_smooth(dst, rad, rad2)
        #define KERNEL_DENSITY_NORM(rad)              kernelDensityNorm_smooth(rad)
        #define KERNEL_DERIV_SCALE(rad)               kernelDerivativeScale_smooth(rad)
    #endif
#endif

#endif