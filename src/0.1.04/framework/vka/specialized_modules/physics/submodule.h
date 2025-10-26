#pragma once
#include <vka/advanced_utility/complex_commands.h>

namespace vka
{
	namespace physics
	{
		struct ParticleDescription;
		class NeighborhoodIterator
		{
		private:
			// Radix sort resources
			Buffer pingPongCellKeys = nullptr;
			Buffer pingPongPermutation = nullptr;
			Buffer histogram = nullptr;
			Buffer cellKeys = nullptr;
			Buffer startIndices = nullptr;
			Buffer permutation = nullptr;
			Buffer uniformBuf = nullptr;

		public:
			bool isInitialized() const
			{
				return cellKeys && startIndices && permutation
					&& pingPongCellKeys && pingPongPermutation && histogram;
			}
			void init(IResourcePool* pPool, uint32_t preallocCount = 0);
			NeighborhoodIterator() = default;
			NeighborhoodIterator(IResourcePool* pPool);


	        void NeighborhoodIterator::cmdUpdate(CmdBuffer cmdBuf, Buffer particleBuf, const ParticleDescription &desc);
			void bind(ComputeCmd& cmd, const ParticleDescription& desc) const;
		};
	}
}