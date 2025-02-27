#ifndef PT_BOX_INTERSECTION_H
#define PT_BOX_INTERSECTION_H

#include "../../lib/min_heap.glsl"

#define BOX_INTERSECT_LIST_SIZE MIN_HEAP_SIZE


#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

layout(binding = PT_BOX_INTERSECTION_BINDING_OFFSET) uniform accelerationStructureEXT boxAs;


//bool foundContainigBox(rayQueryEXT rq, vec3 pos)
//{
//	return (rayQueryGetIntersectionTypeEXT(rq, false) == gl_RayQueryCandidateIntersectionAABBEXT)
//			&& inUnitCube(rayQueryGetIntersectionWorldToObjectEXT(rq, false) * vec4(pos,1.0));
//}

//int[4] containigBoxes(vec3 pos)
//{
//	rayQueryEXT rq;
//	rayQueryInitializeEXT(rq, boxAs, 0, 0xFF, pos, 0, vec3(0.0,1.0,0.0), 0);
//	uint i = 0;
//	int ids[4] = {-1,-1,-1,-1};
//	while(rayQueryProceedEXT(rq))
//	{
//		if(foundContainigBox(rq, pos))
//		{
//			ids[i++] = rayQueryGetIntersectionInstanceCustomIndexEXT(rq, false);;
//		}
//	}
//	return ids;
//}

bool foundIntersectingBox(rayQueryEXT rq, vec3 origin, vec3 direction, out float t)
{
	//if (rayQueryGetIntersectionTypeEXT(rq, false) == gl_RayQueryCandidateIntersectionAABBEXT)
	//{
	//}
	vec3 localPos = rayQueryGetIntersectionWorldToObjectEXT(rq, false) * vec4(origin, 1.0);
	vec3 localDir = rayQueryGetIntersectionWorldToObjectEXT(rq, false) * vec4(direction, 0.0);
	vec3 localEntry, localExit;
	if(unitCubeIntersection(localPos, localDir, localEntry, localExit))
	{
		vec3 entry = rayQueryGetIntersectionObjectToWorldEXT(rq, false) * vec4(localEntry, 1.0);
		t = distance(entry, origin);
		return true;
	}
	return false;
}

int[4] intersectingBoxes(vec3 origin, vec3 direction, float maxLenght)
{
	rayQueryEXT rq;
	rayQueryInitializeEXT(rq, boxAs, 0, 0xFF, origin, TMIN, direction, maxLenght);
	MinHeap ids;
	minHeapInit(ids);
	float t;

	while(rayQueryProceedEXT(rq))
	{
		if(foundIntersectingBox(rq, origin, direction, t))
		{
			int id = rayQueryGetIntersectionInstanceCustomIndexEXT(rq, false);
			minHeapAdd(ids, id, t);
		}
	}
	//int[4] mediumHits = {0, 1, 2, 3};
	//return mediumHits;



	return ids.id;
}

#endif