

struct PCGenerateParticles
{
	vec2 x_range;
	vec2 y_range;
	uint seed;
};

struct GLSLParticle
{
	vec2 pos;
	vec2 vel;
};

struct PCRenderParticles
{
	vec2	viewOffset;
	vec2	viewScale;
	vec2	extent;
	float	pointSize;
	float	intensity;
};

struct PCUpdateParticles
{
	vec2 x_range;
	vec2 y_range;
	float pointSize;
	float dt;
	float damping;
	float gravity;
};