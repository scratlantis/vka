#ifndef LOCAL_STRUCTS_H
#define LOCAL_STRUCTS_H

#ifndef TMIN
#define TMIN 0.0001
#endif

#ifndef TMAX
#define TMAX 1000.0
#endif

struct Ray
{
	vec3 origin;
	vec3 direction;
	float tmin;
	float tmax;
	vec3 weight;
};

struct MaterialData
{
	vec3 albedo;
	vec3 specular;
	vec3 emission;
	float roughness;
	float f0;
};

struct Range
{
	float rMin;
	float rMax;
};


#endif