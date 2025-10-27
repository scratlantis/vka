#include "config.h"

static const float cParticle_size_scale = 0.1f;

struct GenParticleArgs
{
	uint32_t      particleCount;
	Rect2D<float> area;
	uint32_t      seed;
	float         radius;
};

void cmdGenParticles(CmdBuffer cmdBuf, Buffer particleBuffer, Buffer predictedPosBuffer, Buffer velocityBuffer);


struct RenderParticleArgs
{
	float    particleSize;
	float    particleIntensity;
	float	 velocityIntensity;
};

void cmdRenderParticles(CmdBuffer cmdBuf, Image target, Buffer particleBuffer, Buffer densityBuffer);


struct UpdateParticleArgs
{
	Rect2D<float> bounds;
	float         particleSize;
	float         dt;
	float         damping;
	float         dampingBorderCollision;
	float         gravity;
};


struct SimulationResources
{
	physics::NeighborhoodIterator it;
	Buffer                        densityBuffer = nullptr;
	Buffer                        forceBuffer = nullptr;
	void                          init(IResourcePool *pPool, uint32_t preallocCount = 0);
	bool                          isInitialized() const;
};


void cmdSimulateParticles(CmdBuffer cmdBuf, Buffer particleBuffer, Buffer predictedPosBuffer, Buffer velocityBuffer, SimulationResources &res, float timeStep);

    //void cmdUpdateParticleDensity(CmdBuffer cmdBuf, Buffer particleBuffer, physics::NeighborhoodIteratorResources res, Buffer densityBuffer, Buffer forceBuffer);
