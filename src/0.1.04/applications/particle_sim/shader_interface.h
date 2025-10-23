#include "config.h"

struct GenParticleArgs
{
	uint32_t particleCount;
	Rect2D<float> area;
	uint32_t seed;
};

void cmdGenParticles(CmdBuffer cmdBuf, Buffer particleBuffer, GenParticleArgs args);
void cmdGenParticles(CmdBuffer cmdBuf, Buffer particleBuffer);


struct RenderParticleArgs
{
	float    particleSize;
	float    particleIntensity;
};

void cmdRenderParticles(CmdBuffer cmdBuf, Image target, Buffer particleBuffer, RenderParticleArgs args);
void cmdRenderParticles(CmdBuffer cmdBuf, Image target, Buffer particleBuffer);


struct UpdateParticleArgs
{
	Rect2D<float> bounds;
	float particleSize;
	float dt;
	float damping;
	float gravity;
};

void cmdUpdateParticles(CmdBuffer cmdBuf, Buffer particleBuffer, UpdateParticleArgs args);
void cmdUpdateParticles(CmdBuffer cmdBuf, Buffer particleBuffer);