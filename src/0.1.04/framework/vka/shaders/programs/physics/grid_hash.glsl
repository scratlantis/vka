#ifndef GRID_HASH_H
#define GRID_HASH_H
#include "../../lib/random.glsl"

//uint calcCellKey(vec2 pos, float radius, uint range)
//{
//	ivec2 cellOffset = ivec2(floor(pos / (2.0 * radius)));
//	//cellOffset += ivec2(10000); // avoid negative values
//	return hash(cellOffset)%range;
//}
//
//uint calcCellKey(vec3 pos, float radius, uint range)
//{
//	ivec3 cellOffset = ivec3(floor(pos / (2.0 * radius)));
//	return hash(cellOffset)%range;
//}



const ivec2 offsets2D[9] =
{
	ivec2(-1, 1),
	ivec2(0, 1),
	ivec2(1, 1),
	ivec2(-1, 0),
	ivec2(0, 0),
	ivec2(1, 0),
	ivec2(-1, -1),
	ivec2(0, -1),
	ivec2(1, -1),
};

// Constants used for hashing
const uint hashK1 = 15823;
const uint hashK2 = 9737333;

// Convert floating point position into an integer cell coordinate
ivec2 GetCell2D(vec2 position, float radius)
{
	return ivec2(floor(position / radius));
}

// Hash cell coordinate to a single unsigned integer
uint HashCell2D(ivec2 cell)
{
	cell = ivec2(uvec2(cell));
	uint a = cell.x * hashK1;
	uint b = cell.y * hashK2;
	return (a + b);
}

uint KeyFromHash(uint hash, uint tableSize)
{
	return hash % tableSize;
}

uint KeyFromPosition(vec2 position, float radius, uint tableSize)
{
	ivec2 cell = GetCell2D(position, radius);
	uint hash = HashCell2D(cell);
	return KeyFromHash(hash, tableSize);
}

#endif // GRID_HASH_H