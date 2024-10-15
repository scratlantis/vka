#ifndef PT_USCENE_MOCK_H
#define PT_USCENE_MOCK_H

#include "../../lib/pt_common.glsl"

#ifndef BACKGROUND_LIGHTING
#define BACKGROUND_LIGHTING (vec3(1.0, 1.0, 1.0) * 0.8)
#endif

#ifndef AREA_LIGHT_COUNT
#define AREA_LIGHT_COUNT 0
#endif

struct HitData
{
	uint instanceCustomID;
	uint geometryID;
	uint primitiveID;
	vec3 barycentrics;
	mat4x3 objToWorld;
};

GLSLInstance getInstanceData(HitData hitData)
{
	GLSLInstance inst;
	return inst;
}

float traceGeometry(Ray ray, uint cullMask,inout HitData hData)
{
	return TMAX;
}

float traceGeometryTransmittance(Ray ray, uint cullMask)
{
	return 1.0;
}

void evalHit(HitData hitData, inout MaterialData matData, inout mat4x3 tangentFrame){}

Ray genDirectIllumRayEnvMap(vec3 pos, inout uint seed)
{
	Ray ray;
	ray.origin = pos;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	ray.direction = normalize(vec3(0.5) - random3D(seed));
	ray.weight = 4.0 * PI * vec3(BACKGROUND_LIGHTING);
	return ray;
}

vec3 sampleEnvMap(vec3 dir)
{
	return vec3(BACKGROUND_LIGHTING);
}


#endif