
#include "resources.h"
#include "shaders/interface_structs.glsl"

using namespace physics;


void ParticleResources::createTemporaryBuffers()
{
	tmpParticleBuffer = particleMemory->getSubBuffer({0, *pParticleCount * desc.structureSize});
	tmpPredPosBuffer  = predPosMemory->getSubBuffer({0, *pParticleCount * vec_size_aligned(desc.dimensions)});
	tmpVelocityBuffer = velocityMemory->getSubBuffer({0, *pParticleCount * vec_size_aligned(desc.dimensions)});
}

void ParticleResources::refreshTemporaryBuffers()
{
	VKA_ASSERT(isInitialized());
	if (tmpFrameID != *pCurrentFrameID)
	{
		createTemporaryBuffers();
		tmpFrameID = *pCurrentFrameID;
	}
}
const Buffer ParticleResources::getVelocityBuf()
{
	refreshTemporaryBuffers();
	return tmpVelocityBuffer;
}
const Buffer ParticleResources::getParamsBuf()
{
	return paramsBuffer;
}
const Buffer ParticleResources::getPredictedPosBuf()
{
	refreshTemporaryBuffers();
	return tmpPredPosBuffer;
}
const Buffer ParticleResources::getParticleBuf()
{
	refreshTemporaryBuffers();
	return tmpParticleBuffer;
}

void ParticleResources::init(uint32_t maxParticleCount, const ParticleDescription &desc, IResourcePool *pPool, const uint64_t *pFrameID, const uint32_t *pParticleCount)
{
	particleMemory = createBuffer(pPool, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	particleMemory->changeSize(maxParticleCount * desc.structureSize);
	particleMemory->recreate();

	predPosMemory = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	predPosMemory->changeSize(maxParticleCount * vec_size_aligned(desc.dimensions));
	predPosMemory->recreate();

	velocityMemory = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	velocityMemory->changeSize(maxParticleCount * vec_size_aligned(desc.dimensions));
	velocityMemory->recreate();

	this->desc            = desc;
	this->pCurrentFrameID = pFrameID;
	this->pParticleCount  = pParticleCount;

	descAttr = desc;
	descAttr.posAttributeOffset = 0;
	descAttr.velAttributeOffset = 0;
	if (desc.dimensions == PD_3D)
	{
		descAttr.structureSize = vec_size_aligned(PD_3D);
	}
	else
	{
		descAttr.structureSize = vec_size_aligned(PD_2D);
	}

	paramsBuffer = createBuffer(pPool, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	paramsBuffer->changeSize(sizeof(GLSLParticleUpdateParams));
	paramsBuffer->recreate();


	simRes.init(pPool, maxParticleCount);
}