#pragma once
#include "config.h"


extern const float cParticle_size_scale;


FixedCameraState loadCamState();
void saveCamState(FixedCameraState state);

DrawCmd getCmdRenderParticles2D(Image target, Buffer particleBuffer, Buffer densityBuffer, Buffer forceBuffer);
DrawCmd getCmdRenderParticles3D(Image target, Buffer particleBuffer, Buffer densityBuffer, Buffer forceBuffer);

class ParticleResources;

ComputeCmd getCmdGenParticles(ParticleResources *pRes);

struct SimulationResources
{
	physics::NeighborhoodIterator it;
	Buffer                        densityBuffer = nullptr;
	Buffer                        forceBuffer = nullptr;
	void                          init(IResourcePool *pPool, uint32_t preallocCount = 0);
	bool                          isInitialized() const;
};

struct GLSLParticleUpdateParams;
void cmdSimulateParticles(CmdBuffer cmdBuf, ParticleResources *pRes, GLSLParticleUpdateParams params, uint32_t frameInvocationNr);

