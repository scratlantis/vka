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
			VK_FORMAT_R32G32_SFLOAT
		};
		layout.offsets =
		{
			offsetof(GLSLParticle, pos),
			offsetof(GLSLParticle, vel)
		};
		layout.stride = sizeof(GLSLParticle);
		return layout;
	}
};

template <typename Particle>
struct particle_type;

template <>
struct particle_type<GLSLParticle>
{
	static physics::ParticleDescription get_description(float radius)
	{
		physics::ParticleDescription desc{};
		desc.radius = radius;
		desc.dimensions = physics::PD_2D;
		desc.structureSize = sizeof(GLSLParticle);
		desc.posAttributeOffset = offsetof(GLSLParticle, pos);
		return desc;
	}
};


template <>
struct vertex_type<GLSLParticle3D>
{
	VertexDataLayout data_layout()
	{
		VertexDataLayout layout{};
		layout.formats =
		    {
		        VK_FORMAT_R32G32B32_SFLOAT,
		        VK_FORMAT_R32G32B32_SFLOAT};
		layout.offsets =
		    {
		        offsetof(GLSLParticle3D, pos),
		        offsetof(GLSLParticle3D, vel)};
		layout.stride = sizeof(GLSLParticle3D);
		return layout;
	}
};


template<>
struct particle_type<GLSLParticle3D>
{
	static physics::ParticleDescription get_description(float radius)
	{
		physics::ParticleDescription desc{};
		desc.radius = radius;
		desc.dimensions = physics::PD_3D;
		desc.structureSize = sizeof(GLSLParticle3D);
		desc.posAttributeOffset = offsetof(GLSLParticle3D, pos);
		return desc;
	}
};

template <>
struct particle_type<vec2>
{
	static physics::ParticleDescription get_description(float radius)
	{
		physics::ParticleDescription desc{};
		desc.radius = radius;
		desc.dimensions = physics::PD_2D;
		desc.structureSize = sizeof(vec2);
		desc.posAttributeOffset = 0;
		return desc;
	}
};
