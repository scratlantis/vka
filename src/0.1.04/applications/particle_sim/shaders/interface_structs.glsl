#ifndef PARTICLE_INTERFACE_STRUCTS
#define PARTICLE_INTERFACE_STRUCTS


struct GLSLParticle
{
	vec2 pos;
	vec2 vel;
};

#define PARTICLE_STATE_IN_BOX_BIT 0x1

struct GLSLParticle3D
{
	vec3 pos;
	uint stateFlags;

	vec3 vel;
	uint pad1;
};

struct PCGenerateParticles
{
	vec2 x_range;
	vec2 y_range;
	uint seed;
	uint taskSize;
	float radius;
};


struct PCUpdateParticles
{
	vec2 x_range;
	vec2 y_range;
	vec2 mouse_pos;
	uvec2 mouse_buttons;
	float mouse_radius;
	float mouse_influence;
	float pointSize;
	float dt;
	float damping;
	float gravity;
	uint taskSize;
};


#define PARTICLE_EARTH_GRAVITY_BIT 0x1
#define PARTICLE_GRAVITY_BIT 0x2
#define PARTICLE_BOX_BIT 0x4

struct GLSLParticleUpdateParams
{

	mat4 modelMat;
	mat4 modelMatLastFrame;
	mat4 invModelMat;
	mat4 invModelMatLastFrame;
	uint toggleFlags;
	uint frameIdx;
};

#endif