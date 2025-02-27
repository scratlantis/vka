#ifndef COMMON_H
#define COMMON_H
#include "interface_structs.glsl"
#include "local_structs.glsl"

#include "math.glsl"
#include "random.glsl"
#include "sampling.glsl"


Ray reflectLampertDiffuse(MaterialData material, mat4x3 tangentFrame, inout uint seed)
{
	vec3 localDir = sampleCosineWeightedHemisphere(vec2(unormNext(seed), unormNext(seed)));
	Ray ray;
	ray.direction = normalize(tangentFrame * vec4(localDir,0.0));
	ray.origin = tangentFrame[3].xyz;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	ray.weight = material.albedo;
	//ray.weight = (material.albedo / PI) * cos(theta)  /  (cos(theta) / PI);
	return ray;
}

vec3 lampertBRDF(vec3 Kd)
{
	return Kd / PI;
}

vec3 uniformScatterBSDF(vec3 Kd)
{
	return Kd / (4 * PI);
}

Ray scatterUniform(vec3 albedo, vec3 pos, inout uint seed)
{
	vec3 dir = sampleUniformSphere(vec2(unormNext(seed), unormNext(seed)));
	Ray ray;
	ray.direction = normalize(dir);
	ray.origin = pos;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	ray.weight = albedo;
	//ray.weight = (albedo / (4 * PI)) / (1 / (4 * PI));
	return ray;
}

Ray scatterGreenstein(vec3 albedo, vec3 pos, vec3 dir, float g, inout uint seed)
{
	float pdf = 1.0;
	vec3 dir2 = sampleHeneyGreenstein(-dir, g, vec2(unormNext(seed), unormNext(seed)), pdf);
	Ray ray;
	ray.direction = normalize(dir2);
	ray.origin = pos;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	ray.weight = albedo;
	return ray;

}


vec3 sampleAreaLight(vec3 pos, VKAAreaLight light, inout uint seed, out float pdf)
{
	vec2 xi = vec2(unormNext(seed), unormNext(seed));
	vec2 bary = sampleTriangleBarycentrics(xi);
	vec3 samplePos = bary.x * light.v0 + bary.y * light.v1 + (1.0 - bary.x - bary.y) * light.v2;
	vec3 sampleDir = normalize(samplePos - pos);
	float dist = distance(samplePos, pos);
	dist = clamp(dist, 0.05, 100.0);
	float dotSurfaceNormal = absDot(light.normal, -sampleDir);
	dotSurfaceNormal = clamp(dotSurfaceNormal, 0.05, 1.0);
	pdf = dist*dist / (triangleArea(light.v0, light.v1, light.v2) * dotSurfaceNormal);
	return samplePos;
}

#endif