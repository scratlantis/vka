
#include "../interface.glsl"
#include "../random.glsl"
#include "../vka_structs.glsl"
#include "pt_params.glsl"
#include "sampling.glsl"
#include "pt_math.glsl"

#ifndef PT_COMMON_H
#define PT_COMMON_H

struct Ray
{
	vec3 origin;
	vec3 direction;
	float tmin;
	float tmax;
};

struct MaterialData
{
	vec3 albedo;
	vec3 specular;
	vec3 emission;
	float roughness;
	float f0;
};

Ray genPrimaryRay(GLSLFrame frame, GLSLView view, uvec2 pixel, inout uint seed)
{
	Ray ray;
	const vec2 pixelCenter = vec2(gl_GlobalInvocationID.xy) + vec2(unormNext(seed), unormNext(seed));
	const vec2 pixelUV = pixelCenter / vec2(frame.width, frame.height);
	const vec2 d = pixelUV * 2.0 - 1.0;
	ray.origin = (view.invMat * vec4(0,0,0,1)).xyz;
	vec4 target = frame.invProjection * vec4(d.x, d.y, 1, 1) ;
	ray.direction = (view.invMat*vec4(normalize(target.xyz), 0)).xyz ;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	return ray;
}

Ray reflectLampertDiffuse(MaterialData material, mat4x3 tangentFrame, inout uint seed, inout float pdf, inout vec3 weight)
{
	vec3 localDir = sampleCosineWeightedHemisphere(vec2(unormNext(seed), unormNext(seed)));
	pdf *= pdfCosineWeightedHemisphere(localDir);
	weight *= evalCosineWeightedHemisphere(material.albedo);
	Ray ray;
	ray.direction = tangentFrame * vec4(localDir,0.0);
	ray.origin = tangentFrame[3].xyz;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	return ray;

}
#endif