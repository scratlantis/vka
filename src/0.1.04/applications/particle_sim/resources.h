#pragma once
#include "config.h"
#include "shader_interface.h"


class ParticleResources
{
  private:
	Buffer particleMemory = nullptr;
	Buffer predPosMemory  = nullptr;
	Buffer velocityMemory = nullptr;

	const uint64_t *pCurrentFrameID = nullptr;
	const uint32_t *pParticleCount  = nullptr;

	uint64_t tmpFrameID        = 0xFFFFFFFFFFFFFFFF;
	Buffer   tmpParticleBuffer = nullptr;
	Buffer   tmpPredPosBuffer  = nullptr;
	Buffer   tmpVelocityBuffer = nullptr;

	void createTemporaryBuffers();
	void refreshTemporaryBuffers();

  public:
	SimulationResources simRes;
	physics::ParticleDescription desc;

	bool isInitialized() const
	{
		return particleMemory && predPosMemory && velocityMemory && pCurrentFrameID && pParticleCount && simRes.isInitialized();
	}
	ParticleResources() = default;
	DELETE_COPY_CONSTRUCTORS(ParticleResources)
	void init(uint32_t maxParticleCount, const physics::ParticleDescription &desc, IResourcePool *pPool, const uint64_t *pFrameID, const uint32_t *pParticleCount);

	const Buffer getParticleBuf();
	const Buffer getPredictedPosBuf();
	const Buffer getVelocityBuf();

	uint32_t count() const
	{
		VKA_ASSERT(isInitialized());
		return *pParticleCount;
	}
};
