#ifndef LOCAL_STRUCTS_H
#define LOCAL_STRUCTS_H

#ifndef TMIN
#define TMIN 0.00001
#endif

#ifndef TMAX
#define TMAX 1000.0
#endif

const uint PATH_VERTEX_TYPE_LEAF = 0;
const uint PATH_VERTEX_TYPE_LEAF_INFINITY = 1;
const uint PATH_VERTEX_TYPE_SURFACE_SCATTER = 2;
const uint PATH_VERTEX_TYPE_VOLUME_SCATTER = 3;

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