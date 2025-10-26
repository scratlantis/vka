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

void cmdRenderParticles(CmdBuffer cmdBuf, Image target, Buffer particleBuffer, Buffer densityBuffer, RenderParticleArgs args);
void cmdRenderParticles(CmdBuffer cmdBuf, Image target, Buffer particleBuffer, Buffer densityBuffer);


struct UpdateParticleArgs
{
	Rect2D<float> bounds;
	float particleSize;
	float dt;
	float damping;
	float gravity;
};

void cmdUpdateParticles(CmdBuffer cmdBuf, Buffer particleBuffer, Buffer forceBuffer, UpdateParticleArgs args);
void cmdUpdateParticles(CmdBuffer cmdBuf, Buffer particleBuffer, Buffer forceBuffer);
void cmdUpdateParticleDensity(CmdBuffer cmdBuf, Buffer particleBuffer, physics::NeighborhoodIteratorResources res, Buffer densityBuffer, Buffer forceBuffer);
