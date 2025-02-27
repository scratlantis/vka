#ifndef PT_CAMERA_H
#define PT_CAMERA_H

#include "../../lib/local_structs.glsl"
#include "../../lib/random.glsl"
#include "../../lib/default_scene/interface_structs.glsl"
#include "../../lib/compute_shader_base.glsl"

layout(binding = PT_CAMERA_BINDING_OFFSET) uniform CAM									 
{																		 
	GLSLCamera cam;													 
};																		 
layout(binding = PT_CAMERA_BINDING_OFFSET + 1) uniform CAM_INST								 
{																		 
	GLSLCameraInstance camInst;													     
};																		 

Ray genPrimaryRay(uvec2 pixel, inout uint seed)
{
	Ray ray;
	const vec2 pixelCenter = vec2(pixel) + vec2(unormNext(seed), unormNext(seed));
	const vec2 pixelUV = pixelCenter / vec2(cam.width, cam.height);
	const vec2 d = pixelUV * 2.0 - 1.0;
	ray.origin = (camInst.invView * vec4(0,0,0,1)).xyz;
	vec4 target = cam.invProjection * vec4(d.x, d.y, 1, 1) ;
	ray.direction = (camInst.invView*vec4(normalize(target.xyz), 0)).xyz ;
	ray.tmin = TMIN;
	ray.tmax = TMAX;
	ray.weight = vec3(1.0);
	return ray;
}

uint getSeed()
{
	return invocationID() + invocationCount() * camInst.frameIdx;
}

uint pixelID(uvec2 pixel)
{
	return pixel.x + pixel.y * cam.width;
}

uint pixelCount()
{
	return cam.width * cam.height;
}

uint getFrameIdx()
{
	return camInst.frameIdx;
}



#endif