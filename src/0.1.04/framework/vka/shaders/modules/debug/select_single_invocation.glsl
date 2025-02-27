#ifndef SELECT_SINGLE_INVOCATION_H
#define SELECT_SINGLE_INVOCATION_H

layout(binding = SELECT_SINGLE_INVOCATION_BINDING_OFFSET) uniform INVOCATION_SELECTION
{
	vec3 invocationSelection;
};

bool invocationIsSelected()
{
	uvec3 selectedId = uvec3( invocationSelection * vec3(INVOCATION_COUNT_X, INVOCATION_COUNT_Y, INVOCATION_COUNT_Z));
	return gl_GlobalInvocationID == selectedId;
}

#endif