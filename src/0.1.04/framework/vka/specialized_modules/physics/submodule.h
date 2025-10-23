#pragma once
#include <vka/advanced_utility/complex_commands.h>

namespace vka
{
	namespace physics
	{
		struct ParticleDescription;
		struct NeighborhoodIteratorResources
		{
			Buffer cellKeys = nullptr;
			Buffer startIndices = nullptr;
			Buffer permutation = nullptr;
			// Radix sort resources
			Buffer pingPongCellKeys = nullptr;
			Buffer pingPongPermutation = nullptr;
			Buffer histogram = nullptr;

			bool isInitialized() const
			{
				return cellKeys && startIndices && permutation
					&& pingPongCellKeys && pingPongPermutation && histogram;
			}
			void init(IResourcePool* pPool);
			NeighborhoodIteratorResources() = default;
			NeighborhoodIteratorResources(IResourcePool* pPool);
		};
		void cmdUpdateNeighborhoodIterator(CmdBuffer cmdBuf, Buffer particleBuf, const ParticleDescription& desc, NeighborhoodIteratorResources& res);
		void bindNeighborhoodIterator(ComputeCmd& cmd, NeighborhoodIteratorResources res);
	}
}