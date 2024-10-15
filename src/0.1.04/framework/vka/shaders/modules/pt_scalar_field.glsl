#ifndef PT_SCALAR_FIELD_H
#define PT_SCALAR_FIELD_H

#include "../lib/pt_common.glsl"

#ifndef VOLUME_RESOLUTION
#define VOLUME_RESOLUTION 128
#endif

#ifndef RAY_MARCHE_STEP_SIZE
#define RAY_MARCHE_STEP_SIZE 0.2
#endif

#define PT_SCALAR_FIELD_BINDING_COUNT 1
layout(binding = PT_SCALAR_FIELD_BINDING_OFFSET) uniform sampler3D volSmp;

float rayMarcheMedium(vec3 origin, vec3 direction, float maxLenght, inout uint seed)
{
	float rng = unormNext(seed);
	const float max_steps = sqrt(3.0) * 2.0 / RAY_MARCHE_STEP_SIZE;
	float t = 0.0;
	float transmittance = 1.0;
	float stepSize = RAY_MARCHE_STEP_SIZE * unormNext(seed);
	stepSize = min(stepSize, maxLenght);

	for(uint i = 0; i < max_steps; i++)
	{
		t += stepSize;
		vec3 pos = origin + direction * (t-stepSize*unormNext(seed));

		// Sample density
		float density = texture(volSmp, pos).r;
		transmittance *= exp(-density * stepSize);

		// Decide if we should stop
		if(transmittance < rng)
		{
			return t;
		}

		stepSize = min(RAY_MARCHE_STEP_SIZE, maxLenght - t);
		if( stepSize < EPSILON)
		{
			return TMAX;
		}
	}
	return TMAX;
}

float rayMarcheMediumTransmittance(vec3 origin, vec3 direction, float maxLenght, inout uint seed)
{
	const float max_steps = sqrt(3.0) * 2.0 / RAY_MARCHE_STEP_SIZE;
	float t = 0.0;
	float transmittance = 1.0;
	float stepSize = RAY_MARCHE_STEP_SIZE * unormNext(seed);
	stepSize = min(stepSize, maxLenght);
	for(uint i = 0; i < max_steps; i++)
	{
		t += stepSize;
		//vec3 pos = origin + direction * (t-stepSize*unormNext(seed));
		vec3 pos = origin + direction * (t-stepSize*0.5);
		//vec3 pos = origin + direction * t;

		// Sample density
		float density = texture(volSmp, pos).r;
		transmittance *= exp(-density * stepSize);

		stepSize = min(RAY_MARCHE_STEP_SIZE, maxLenght - t);
		if( stepSize < EPSILON)
		{
			return transmittance;
		}
	}
	return transmittance;
}

#endif