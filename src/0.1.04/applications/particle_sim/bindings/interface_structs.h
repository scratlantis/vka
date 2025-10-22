#pragma once
#include "config.h"
using namespace glm;
#include "../shaders/interface_structs.glsl"

template <>
struct vertex_type<GLSLParticle>
{
	VertexDataLayout data_layout()
	{
		VertexDataLayout layout{};
		layout.formats =
		{
			VK_FORMAT_R32G32_SFLOAT,
		};
		layout.offsets =
		{
			offsetof(GLSLParticle, pos)
		};
		layout.stride = sizeof(GLSLParticle);
		return layout;
	}
};