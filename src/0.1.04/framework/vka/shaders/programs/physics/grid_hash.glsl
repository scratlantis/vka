#ifndef GRID_HASH_H
#define GRID_HASH_H
#include "../../lib/random.glsl"

// https://github.com/SebLague/Fluid-Sim/tree/Episode-01

uint keyFromHash(uint hash, uint tableSize)
{
	return hash % tableSize;
}

// Constants used for hashing
const uint hashK1 = 15823;
const uint hashK2 = 9737333;
const uint hashK3 = 541523;


// 2D
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

ivec2 applyOffset(ivec2 cell, uint index)
{
	return cell + offsets2D[index];
}


// Convert floating point position into an integer cell coordinate
ivec2 getCell(vec2 position, float radius)
{
	return ivec2(floor(position / radius));
}

// Hash cell coordinate to a single unsigned integer
uint hashCell(ivec2 cell)
{
	cell = ivec2(uvec2(cell));
	uint a = cell.x * hashK1;
	uint b = cell.y * hashK2;
	return (a + b);
}


uint keyFromPosition(vec2 position, float radius, uint tableSize)
{
	ivec2 cell = getCell(position, radius);
	uint hash = hashCell(cell);
	return keyFromHash(hash, tableSize);
}

// 3D
const ivec3 offsets3D[27] =
{
	ivec3(-1, 1, -1),
	ivec3(0, 1, -1),
	ivec3(1, 1, -1),
	ivec3(-1, 0, -1),
	ivec3(0, 0, -1),
	ivec3(1, 0, -1),
	ivec3(-1, -1, -1),
	ivec3(0, -1, -1),
	ivec3(1, -1, -1),

	ivec3(-1, 1, 0),
	ivec3(0, 1, 0),
	ivec3(1, 1, 0),
	ivec3(-1, 0, 0),
	ivec3(0, 0, 0),
	ivec3(1, 0, 0),
	ivec3(-1, -1, 0),
	ivec3(0, -1, 0),
	ivec3(1, -1, 0),

	ivec3(-1, 1, 1),
	ivec3(0, 1, 1),
	ivec3(1, 1, 1),
	ivec3(-1, 0, 1),
	ivec3(0, 0, 1),
	ivec3(1, 0, 1),
	ivec3(-1, -1, 1),
	ivec3(0, -1, 1),
	ivec3(1, -1, 1),
};

ivec3 applyOffset(ivec3 cell, uint index)
{
	return cell + offsets3D[index];
}

ivec3 getCell(vec3 position, float radius)
{
	return ivec3(floor(position / radius));
}

uint hashCell(ivec3 cell)
{
	cell = ivec3(uvec3(cell));
	uint a = cell.x * hashK1;
	uint b = cell.y * hashK2;
	uint c = cell.z * hashK3;
	return (a + b + c);
}

uint keyFromPosition(vec3 position, float radius, uint tableSize)
{
	ivec3 cell = getCell(position, radius);
	uint hash = hashCell(cell);
	return keyFromHash(hash, tableSize);
}


#endif // GRID_HASH_H