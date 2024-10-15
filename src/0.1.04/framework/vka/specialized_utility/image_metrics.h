#pragma once
#include "compute.h"
#include "draw_2D.h"

namespace vka
{
	struct MSEComputeResources
	{
		private:
			VkFormat format;
			IResourcePool *pPool;
		public:
		Image  diffImage = nullptr;
		Buffer verticalAverage = nullptr;
		Buffer mse = nullptr;
	    MSEComputeResources() = default;
	    ~MSEComputeResources() = default;
		MSEComputeResources(VkFormat format, IResourcePool *pPool) :
	        format(format),
	        pPool(pPool)
	    {}

		void update(VkExtent2D extent)
		{
			if (!diffImage)
			{
				diffImage = createImage(pPool, format, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, extent);
			}
		    else
		    {
			    diffImage->changeExtent(VkExtent3D{extent.width, extent.height, 1});
				diffImage->recreate();

		    }
			if (!verticalAverage)
			{
				verticalAverage = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, extent.width * sizeof(float));
			}
		    else
		    {
				verticalAverage->changeSize(extent.width * sizeof(float));
				verticalAverage->recreate();
		    }
			if (!mse)
			{
			    mse = createBuffer(pPool, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(float));
			}
		}
	};
    void cmdComputeMSE(CmdBuffer cmdBuf, Image srcA, Image srcB, Buffer dst, MSEComputeResources *pMSEResources);
    }        // namespace vka