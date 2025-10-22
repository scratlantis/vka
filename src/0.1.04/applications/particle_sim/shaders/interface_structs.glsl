

struct PCGenerateParticles
{
	vec2 x_range;
	vec2 y_range;
	uint seed;
};

struct GLSLParticle
{
	vec2 pos;
	vec2 placeholder;
};

struct PCRenderParticles
{
	vec2	viewOffset;
	vec2	viewScale;
	float	pointSize;
};