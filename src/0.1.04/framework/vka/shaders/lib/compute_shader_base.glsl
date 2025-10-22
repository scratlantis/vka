#ifndef COMPUTE_SHADER_BASE_H
#define COMPUTE_SHADER_BASE_H

layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z_id = 2) in;

#ifndef INVOCATION_COUNT_X
#define INVOCATION_COUNT_X 1
#endif

#ifndef INVOCATION_COUNT_Y
#define INVOCATION_COUNT_Y 1
#endif

#ifndef INVOCATION_COUNT_Z
#define INVOCATION_COUNT_Z 1
#endif

#ifdef USE_SPEC_CONST
layout(constant_id = 3) const uint invCountX = 0;
layout(constant_id = 4) const uint invCountY = 0;
layout(constant_id = 5) const uint invCountZ = 0;

bool validInvocation()
{
	uvec3 gID = gl_GlobalInvocationID;
	return gID.x < invCountX && gID.y < invCountY && gID.z < invCountZ;
}
uint invocationID()
{
	uvec3 gID = gl_GlobalInvocationID;
	return gID.x + gID.y * invCountX + gID.z * invCountX * invCountY;
}
uint invocationCount()
{
	return invCountX * invCountY * invCountZ;
}
#else
bool validInvocation()
{
	uvec3 gID = gl_GlobalInvocationID;
	return gID.x < INVOCATION_COUNT_X && gID.y < INVOCATION_COUNT_Y && gID.z < INVOCATION_COUNT_Z;
}

uint invocationID()
{
	uvec3 gID = gl_GlobalInvocationID;
	return gID.x + gID.y * INVOCATION_COUNT_X + gID.z * INVOCATION_COUNT_X * INVOCATION_COUNT_Y;
}

uint invocationCount()
{
	return INVOCATION_COUNT_X * INVOCATION_COUNT_Y * INVOCATION_COUNT_Z;
}
#endif

#ifdef DEBUG
#define IF_DEBUG( A ) A
#else
#define IF_DEBUG( A )
#endif


#endif