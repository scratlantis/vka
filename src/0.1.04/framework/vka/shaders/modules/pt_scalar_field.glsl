#ifndef PT_SCALAR_FIELD_H
#define PT_SCALAR_FIELD_H

#include "../lib/pt_common.glsl"

#ifndef VOLUME_RESOLUTION
#define VOLUME_RESOLUTION 128
#endif

#ifndef RAY_MARCHE_STEP_SIZE
#define RAY_MARCHE_STEP_SIZE 0.1
#endif

#define PT_SCALAR_FIELD_BINDING_COUNT 1
layout(binding = PT_SCALAR_FIELD_BINDING_OFFSET) uniform sampler3D volSmp;
layout(binding = PT_SCALAR_FIELD_BINDING_OFFSET + 1) uniform DENSITY_SCALE
{
	float densityScale;
	float minDensity;
	float maxDensity;
};

#if 0
float rayMarcheMedium(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	float rng = unormNext(seed);
	const float max_steps = sqrt(3.0) * 2.0 / RAY_MARCHE_STEP_SIZE;
	float t = 0.0;
	float transmittance = 1.0;
	float stepSize = RAY_MARCHE_STEP_SIZE * unormNext(seed);
	stepSize = min(stepSize, maxLength);

	for(uint i = 0; i < max_steps; i++)
	{
		t += stepSize;
		vec3 pos = origin + direction * t;

		// Sample density
		float density = texture(volSmp, pos).r;
		if(density >= minDensity && density < maxDensity)
		{
			density *= densityScale;
			transmittance *= exp(-density * stepSize);
		}

		// Decide if we should stop
		if(transmittance < rng)
		{
			return t;
		}

		stepSize = min(RAY_MARCHE_STEP_SIZE, maxLength - t);
		if( stepSize < EPSILON)
		{
			return TMAX;
		}
	}
	return TMAX;
}
#else
float rayMarcheMedium(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	float rng = unormNext(seed);
	const uint max_steps = uint(SQRT3 / RAY_MARCHE_STEP_SIZE) + 1; // sqrt(3.0) is the maximum distance between two points in a cube
	float t = -RAY_MARCHE_STEP_SIZE * unormNext(seed);
	float transmittance = 1.0;
	for(uint i = 0; i < max_steps; i++)
	{
		t += RAY_MARCHE_STEP_SIZE;
		if(t > maxLength)
		{
			return TMAX;
		}
		vec3 pos = origin + direction * t;

		// Sample density
		float density = texture(volSmp, pos).r;
		if(density >= minDensity && density < maxDensity)
		{
			density*=densityScale;
			transmittance *= exp(-density * RAY_MARCHE_STEP_SIZE);
		}
		// Decide if we should stop
		if(transmittance < rng)
		{
			return t;
		}
	}
	return TMAX;
}
#endif


#if 0
float rayMarcheMediumTransmittance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	const float max_steps = sqrt(3.0) * 2.0 / RAY_MARCHE_STEP_SIZE;
	float t = 0.0;
	float transmittance = 1.0;
	float stepSize = RAY_MARCHE_STEP_SIZE * unormNext(seed);
	stepSize = min(stepSize, maxLength);
	for(uint i = 0; i < max_steps; i++)
	{
		t += stepSize;
		vec3 pos = origin + direction * t;

		// Sample density
		float density = texture(volSmp, pos).r;
		if(density >= minDensity)
		{
			density *= densityScale;
			transmittance *= exp(-density * stepSize);
		}

		stepSize = min(RAY_MARCHE_STEP_SIZE, maxLength - t);
		if( stepSize < 0.00001)
		{
			return transmittance;
		}
	}
	return transmittance;
}
#else
float rayMarcheMediumTransmittance(vec3 origin, vec3 direction, float maxLength, inout uint seed)
{
	const uint max_steps = uint(SQRT3 / RAY_MARCHE_STEP_SIZE) + 1;
	float t = -RAY_MARCHE_STEP_SIZE * unormNext(seed);
	float densitySum = 0.0;
	uint sampleCount = 0;
	for(uint i = 0; i < max_steps; i++)
	{
		t += RAY_MARCHE_STEP_SIZE;
		if(t > maxLength)
		{
			break;
		}
		vec3 pos = origin + direction * t;

		// Sample density
		float density = texture(volSmp, pos).r * densityScale;
		densitySum += density;
		sampleCount++;
	}
	if(sampleCount == 0)
	{
		return 1.0;
	}
	else
	{
		densitySum /= float(sampleCount);
		return exp(-densitySum * maxLength);
	}
}
#endif

#endif