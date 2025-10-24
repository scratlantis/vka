#ifndef GRID_HASH_H
#define GRID_HASH_H
#include "../../lib/random.glsl"

uint calcCellKey(vec2 pos, float radius, uint range)
{
	uvec2 cellOffset = uvec2(floor(pos / (2.0 * radius)));
	return hash(cellOffset)%range;
}

uint calcCellKey(vec3 pos, float radius, uint range)
{
	uvec3 cellOffset = uvec3(floor(pos / (2.0 * radius)));
	return hash(cellOffset)%range;
}
#endif // GRID_HASH_H