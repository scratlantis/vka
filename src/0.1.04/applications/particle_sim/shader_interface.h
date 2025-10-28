#pragma once
#include "config.h"


extern const float cParticle_size_scale;




DrawCmd getCmdRenderParticles2D(Image target, Buffer particleBuffer, Buffer densityBuffer);

class ParticleResources;

ComputeCmd getCmdGenParticles(ParticleResources *pRes);

struct SimulationResources
{
	physics::NeighborhoodIterator it;
	Buffer                        densityBuffer = nullptr;
	Buffer                        forceBuffer = nullptr;
	void                          init(IResourcePool *pPool, uint32_t preallocCount = 0);
	bool                          isInitialized() const;
	void                          SimulationResources::cmdReset(CmdBuffer cmdBuf, uint32_t particleCount, physics::ParticleDimensions dim);
};

void cmdSimulateParticles(CmdBuffer cmdBuf, ParticleResources *pRes);


