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
	float         particleSize;
	float         dt;
	float         damping;
	float         dampingBorderCollision;
	float         gravity;
};

ComputeCmd getCmdUpdateParticles(Buffer particleBuffer, Buffer forceBuffer, UpdateParticleArgs args);

struct SimulationResources
{
	physics::NeighborhoodIterator it;
	Buffer                        densityBuffer = nullptr;
	Buffer                        pressureForceBuffer = nullptr;
	void                          init(IResourcePool *pPool, uint32_t preallocCount = 0);
	bool                          isInitialized() const;
};

void cmdSimulateParticles(CmdBuffer cmdBuf, Buffer particleBuffer, const physics::ParticleDescription &desc, SimulationResources &res, float timeStep);

template<typename T>
void cmdSimulateParticles(CmdBuffer cmdBuf, Buffer particleBuffer, SimulationResources& res, float timeStep)
{
	ParticleDescription desc = particle_type<T>::get_description(gvar_particle_size.val.v_float);
	cmdSimulateParticles(cmdBuf, particleBuffer, desc, res, timeStep);
}


//void cmdUpdateParticleDensity(CmdBuffer cmdBuf, Buffer particleBuffer, physics::NeighborhoodIteratorResources res, Buffer densityBuffer, Buffer forceBuffer);
