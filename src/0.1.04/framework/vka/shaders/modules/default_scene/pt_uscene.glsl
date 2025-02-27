
#ifndef PT_USCENE_H
#define PT_USCENE_H

#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

#include "../../lib/pt_common.glsl"

#ifdef ENVMAP_PDF_BINS_X
	#ifdef ENVMAP_PDF_BINS_Y
		#define USE_ENVMAP
	#endif
#endif

#ifndef BACKGROUND_LIGHTING
#define BACKGROUND_LIGHTING (vec3(1.0, 1.0, 1.0) * 0.8)
#endif

#ifndef AREA_LIGHT_COUNT
#define AREA_LIGHT_COUNT 0
#endif


#define PT_USCENE_BINDING_COUNT 14
layout(binding = PT_USCENE_BINDING_OFFSET + 0) readonly buffer VERTICES { GLSLVertex vertices[]; };
layout(binding = PT_USCENE_BINDING_OFFSET + 1) readonly buffer INDICES { uint indices[]; };
layout(binding = PT_USCENE_BINDING_OFFSET + 2) readonly buffer MODEL_OFFSETS { VKAModelDataOffset modelOffsets[]; };
layout(binding = PT_USCENE_BINDING_OFFSET + 3) readonly buffer SURFACE_OFFSETS { uint surfaceIndexOffsets[]; };
layout(binding = PT_USCENE_BINDING_OFFSET + 4) readonly buffer MATERIALS { GLSLMaterial materials[]; };
layout(binding = PT_USCENE_BINDING_OFFSET + 5) readonly buffer AREA_LIGHTS { VKAAreaLight areaLights[]; };
layout(binding = PT_USCENE_BINDING_OFFSET + 6) uniform accelerationStructureEXT as;
layout(binding = PT_USCENE_BINDING_OFFSET + 7) readonly buffer INSTANCES {GLSLInstance instances[];};
layout(binding = PT_USCENE_BINDING_OFFSET + 8) readonly buffer INSTANCE_OFFSET {uint instanceOffsets[];};
layout(binding = PT_USCENE_BINDING_OFFSET + 9) uniform sampler smp;
layout(binding = PT_USCENE_BINDING_OFFSET + 10) uniform texture2D tex[];
layout(binding = PT_USCENE_BINDING_OFFSET + 11) uniform sampler2D envMap;
layout(binding = PT_USCENE_BINDING_OFFSET + 12) readonly buffer ENV_MAP_PDF {float envMapPdf[];};
//layout(binding = PT_USCENE_BINDING_OFFSET + 13) uniform accelerationStructureEXT otherAs;
//layout(binding = PT_USCENE_BINDING_OFFSET + 13) readonly buffer TEST123 { GLSLVertex vertices123[]; };

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
	return instances[hitData.instanceCustomID];
}

float traceGeometry(Ray ray, uint cullMask, inout HitData hData)
{
	rayQueryEXT rq;
	rayQueryInitializeEXT(rq, as, gl_RayFlagsOpaqueEXT, cullMask, ray.origin, ray.tmin, ray.direction, ray.tmax);
	while(rayQueryProceedEXT(rq)){}
	if(rayQueryGetIntersectionTypeEXT(rq, true) == gl_RayQueryCommittedIntersectionTriangleEXT)
	{
		hData.instanceCustomID = rayQueryGetIntersectionInstanceCustomIndexEXT(rq, true);
		hData.geometryID = rayQueryGetIntersectionGeometryIndexEXT(rq, true);
		hData.primitiveID = rayQueryGetIntersectionPrimitiveIndexEXT(rq, true);
		hData.barycentrics.yz = rayQueryGetIntersectionBarycentricsEXT(rq, true);
		hData.barycentrics.x = 1.0 - hData.barycentrics.y - hData.barycentrics.z;
		hData.objToWorld = rayQueryGetIntersectionObjectToWorldEXT(rq, true);
		return rayQueryGetIntersectionTEXT(rq, true);
	}
	return TMAX;
}

float traceGeometryTransmittance(Ray ray, uint cullMask)
{
	rayQueryEXT rq;
	float transmittance;
	rayQueryInitializeEXT(rq, as, gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT , cullMask, ray.origin, ray.tmin, ray.direction, ray.tmax);
	while(rayQueryProceedEXT(rq)){}
	if(rayQueryGetIntersectionTypeEXT(rq, true) == gl_RayQueryCommittedIntersectionTriangleEXT)
	{
		transmittance = 0.0;
	}
	else
	{
		transmittance = 1.0;
	}
	return transmittance;
}

void evalHit(HitData hitData, inout MaterialData matData, inout mat4x3 tangentFrame)
{
	uint instanceOffset = instanceOffsets[hitData.instanceCustomID];
	VKAModelDataOffset modelOffset = modelOffsets[instanceOffset];
	uint firstIndex = surfaceIndexOffsets[modelOffset.firstSurface + hitData.geometryID] + hitData.primitiveID * 3;
	GLSLVertex v0 = vertices[indices[firstIndex    ] + modelOffset.firstVertex];
	GLSLVertex v1 = vertices[indices[firstIndex + 1] + modelOffset.firstVertex];
	GLSLVertex v2 = vertices[indices[firstIndex + 2] + modelOffset.firstVertex];
	vec2 uv = mat3x2(v0.uv, v1.uv, v2.uv) * hitData.barycentrics;

	// Material
	GLSLMaterial mat = materials[modelOffset.firstSurface + hitData.geometryID];
	// Add texture reads later
	matData.albedo = mat.albedo;
	//matData.albedo = vec3(0.5);
	matData.specular = mat.specular;
	matData.emission = mat.emission;
	matData.roughness = mat.roughness;
	matData.f0 = mat.f0;

	// Compute tangent frame
	vec3 localPos = mat3(v0.pos, v1.pos, v2.pos) * hitData.barycentrics;
	vec3 localGeometricNormal = normalize(mat3(v0.normal, v1.normal, v2.normal) * hitData.barycentrics);
	vec3 worldPos = hitData.objToWorld * vec4(localPos, 1.0);
	vec3 worldNormal = normalize(hitData.objToWorld * vec4(localGeometricNormal, 0.0));
	tangentFrame = tangentFrameFromPosNormal(worldPos,worldNormal);
}


#if AREA_LIGHT_COUNT > 0
VKAAreaLight selectAreaLight(inout uint seed, out float localPdf)
{
	VKAAreaLight areaLight;
	uint lightIdx = uint(AREA_LIGHT_COUNT * unormNext(seed));
	localPdf = 1.0/float(AREA_LIGHT_COUNT); // uniform pdf
	areaLight = areaLights[lightIdx];
	mat4 transform = instances[areaLight.instanceIndex].mat;
	areaLight.v0 = vec3(transform*vec4(areaLight.v0, 1.0));
	areaLight.v1 = vec3(transform*vec4(areaLight.v1, 1.0));
	areaLight.v2 = vec3(transform*vec4(areaLight.v2, 1.0));
	areaLight.normal = normalize(vec3(transform*vec4(areaLight.normal, 0.0)));
	return areaLight;
}

Ray genDirectIllumRayAreaLight(vec3 pos, inout uint seed)
{
	float selectionPDF, samplePDF;
	VKAAreaLight areaLight = selectAreaLight(seed, selectionPDF);
	vec3 samplePos  = sampleAreaLight(pos, areaLight, seed, samplePDF);
	float pdf = selectionPDF * samplePDF;

	Ray ray;
	ray.origin = pos;
	ray.direction = normalize(samplePos-ray.origin);
	ray.tmin = TMIN;
	ray.tmax = distance(samplePos, ray.origin)-TMIN;
	ray.weight = areaLight.color * vec3(areaLight.intensity) / pdf;
	return ray;
}
#endif

#ifdef USE_ENVMAP
vec2 sampleEnvMapPdf(inout uint seed, out float pdf)
{
	vec4 rng = vec4(unormNext(seed), unormNext(seed), unormNext(seed), unormNext(seed));
	int x = 0;
	int y = 0;

	float cdf = 0;
	float pdfRow;
	for(; y < ENVMAP_PDF_BINS_Y; y++)
	{
		pdfRow = envMapPdf[y];
		cdf += pdfRow;
		if(rng.y < cdf)
		{
			break;
		}
	}

	cdf = 0;
	float pdfColumn;
	for(; x < ENVMAP_PDF_BINS_X; x++)
	{
		pdfColumn = envMapPdf[ENVMAP_PDF_BINS_Y + y * ENVMAP_PDF_BINS_X + x];
		cdf += pdfColumn;
		if(rng.x < cdf)
		{
			break;
		}
	}
	
	float selectionPDF = pdfRow * pdfColumn;
	pdf = selectionPDF * float(ENVMAP_PDF_BINS_X * ENVMAP_PDF_BINS_Y);


	vec2 uv = vec2((float(x)+rng.z)/ENVMAP_PDF_BINS_X, (float(y)+rng.w)/ENVMAP_PDF_BINS_Y);

	return uv;
}
#endif


Ray genDirectIllumRayEnvMap(vec3 pos, inout uint seed)
{
	Ray ray;
	ray.origin = pos;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
#ifdef USE_ENVMAP
	float pdf;
	vec2 uv = sampleEnvMapPdf(seed, pdf);
	ray.direction = uvToDir(uv);
	ray.weight = 4.0 * PI * texture(envMap, uv).rgb / pdf;
#else
	ray.direction = normalize(vec3(0.5) - random3D(seed));
	ray.weight = 4.0 * PI * vec3(BACKGROUND_LIGHTING);
#endif
	return ray;
}

vec3 sampleEnvMap(vec3 dir)
{
	return texture(envMap, dirToUv(dir)).rgb;
}

#endif