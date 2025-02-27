#include "math.glsl"
#ifndef SAMPLING_H
#define SAMPLING_H

vec2 sampleTriangleBarycentrics(vec2 xi) {
	float su0 = sqrt(xi.x);
	return vec2(1 - su0, xi.y * su0);
}

vec3 sampleCosineWeightedHemisphere(vec2 xi)
{
	float xi_0_sqrt = sqrt(xi.x);
	float phi = 2.0f*PI*xi.y;
	return normalize(vec3(cos(phi) * xi_0_sqrt, sin(phi) * xi_0_sqrt, sqrt(1.0f - xi.x)));
}

vec3 sampleUniformSphere(vec2 xi) {
	float theta = 2.0f * PI * xi.x;
	float phi = acos(2.0f * xi.y - 1.0f);
	return normalize(vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi)));
}

float henyeyGreenstein(float cosTheta, float g)
{
	float denom = 1.0 + g*g + 2 * g * cosTheta;
	return INV_4PI * (1.0 - g*g) / (denom * sqrt(max(0.0,denom)));
}


vec3 sampleHeneyGreenstein(vec3 wo, float g, vec2 xi, out float pdf)
{
	float cosTheta = 0.0;
	if (abs(g) < 0.001)
	{
		cosTheta = 1.0 - 2.0 * xi.x;
	}
	else
	{
		float g2 = g * g;
		float sqrTerm = (1.0 - g2) / (1.0 + g - 2.0 * g * xi.x);
		cosTheta = (-1.0 / (2.0 * g)) * (1.0 + g2 - sqrTerm*sqrTerm);
	}
	float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));
	float phi = 2.0 * PI * xi.y;
	vec3 wi = normalize(vec3(sinTheta * cos(phi), cosTheta * sin(phi), cosTheta));
	wi = tangentFrameFromNormal(wo)*wi.xyz;
	pdf = henyeyGreenstein(cosTheta, g);
	return wi;
}

#endif