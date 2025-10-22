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
};

void cmdRenderParticles(CmdBuffer cmdBuf, Image target, Buffer particleBuffer, RenderParticleArgs args);
void cmdRenderParticles(CmdBuffer cmdBuf, Image target, Buffer particleBuffer);