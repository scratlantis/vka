

#include "pt_math.glsl"
#ifndef SAMPLING_H
#define SAMPLING_H

// pbrt

vec2 sampleTriangleBarycentrics(vec2 xi) {
	float su0 = sqrt(xi.x);
	return vec2(1 - su0, xi.y * su0);
}

vec3 sampleCosineWeightedHemisphere(vec2 xi)
{
	float xi_0_sqrt = sqrt(xi.x);
	float phi = 2.0f*PI*xi.y;
	return vec3(cos(phi) * xi_0_sqrt, sin(phi) * xi_0_sqrt, sqrt(1.0f - xi.x));
}

float pdfCosineWeightedHemisphere(vec3 w) {
	return w.z * INV_PI;
}

vec3 evalCosineWeightedHemisphere(vec3 kd) {
	return kd * INV_PI;
}

#endif