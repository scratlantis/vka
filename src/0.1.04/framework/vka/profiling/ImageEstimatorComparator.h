#pragma once
#include <vka/advanced_utility/complex_commands.h>
#include "TimeQueryManager.h"
#include <vka/specialized_utility/image_metrics.h>

namespace vka
{
struct IECToneMappingArgs
{
	bool  useTonemapping;
	float whitePoint;
	float exposure;
	bool  useGammaCorrection;
};
enum IECTarget
{
	IEC_TARGET_LEFT  = 0,
	IEC_TARGET_RIGHT = 1
};
enum IECRunFlagBits
{
	IEC_FLAGS_NONE = 0x00,
	IEC_RUN_NO_MSE = 0x01,
};
class ImageEstimatorComparator
{
  public:
	ImageEstimatorComparator()  = default;
	~ImageEstimatorComparator() = default;
	ImageEstimatorComparator(VkFormat format, float relWidth, float relHeight);

	ImageEstimatorComparator(VkFormat format, VkExtent2D extent);

	void garbageCollect();

	void cmdReset(CmdBuffer cmdBuf, Image imgLeft = nullptr, Image imgRight = nullptr);
	void ImageEstimatorComparator::cmdReset(CmdBuffer cmdBuf, IECTarget target);
	template <class EstimatorArgs>
	void cmdRunEqualRate(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs argsLeft, EstimatorArgs argsRight,
	                     float *timigsLeft = nullptr, float *timigsRight = nullptr, uint32_t flags = 0);

	template <class EstimatorArgs>
	void cmdRun(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs args, IECTarget target, float *timings = nullptr, uint32_t flags = 0);

	template <class EstimatorArgs>
	void cmdRunEqualTime(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs argsLeft, EstimatorArgs argsRight,
	                     float *timigsLeft = nullptr, float *timigsRight = nullptr, uint32_t flags = 0);

	void     cmdShow(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea, VkImageLayout targetLayout, IECTarget src, IECToneMappingArgs toneMappingArgs = {});
	void     showSplitView(CmdBuffer cmdBuf, Image target, float splittCoef, VkRect2D_OP targetArea, IECToneMappingArgs toneMappingArgs = {});
	void     showDiff(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea);
	void     showDiff(CmdBuffer cmdBuf, Image target);
	Buffer   getMSEBuf();
	Buffer   getRMSEBuf();
	float    getMSE();
	float   *getMSEData();
	uint32_t getMSEDataSize();

	struct Metrics
	{
		float    totalTiming;
		uint32_t totalTimeSamples;
		uint32_t invocationCount;

		bool hasTiming() const
		{
			return totalTimeSamples > 0;
		}

		float getAvgTiming() const
		{
			if (totalTimeSamples == 0)
			{
				return 0.0f;
			}
			return totalTiming / float(totalTimeSamples);
		}

		float getTotalRuntime() const
		{
			return getAvgTiming() * float(invocationCount);
		}
		void reset()
		{
			totalTiming      = 0.0f;
			totalTimeSamples = 0;
			invocationCount  = 0;
		}

		void addTiming(float timing)
		{
			totalTiming += timing;
			totalTimeSamples++;
		}
	};

	Image               localTargetLeft, localTargetRight, localAccumulationTargetLeft, localAccumulationTargetRight;
  private:
	TimeQueryManager    tqManagerLeft, tqManagerRight;
	bool                timeQueryFinishedLeft, timeQueryFinishedRight;
	MSEComputeResources mseResources;
	MSEComputeResources rmseResources;
	Buffer              mseBuffer;
	Buffer              rmseBuffer;
	Buffer              mseOverTimeBuffer;
	Buffer              rmseOverTimeBuffer;

	bool                isInitialized = false;

	Metrics            metricsLeft, metricsRight;
	std::vector<float> mse;
};
template <class EstimatorArgs>
inline void ImageEstimatorComparator::cmdRun(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs args, IECTarget target, float *timings, uint32_t flags)
{
	Image                             &localTarget             = target == IEC_TARGET_LEFT ? localTargetLeft : localTargetRight;
	Image                             &localAccumulationTarget = target == IEC_TARGET_LEFT ? localAccumulationTargetLeft : localAccumulationTargetRight;
	ImageEstimatorComparator::Metrics &metrics                 = target == IEC_TARGET_LEFT ? metricsLeft : metricsRight;
	TimeQueryManager                  &tqManager               = target == IEC_TARGET_LEFT ? tqManagerLeft : tqManagerRight;

	cmdFill(cmdBuf, localTarget, VK_IMAGE_LAYOUT_GENERAL, glm::vec4(0.0f));
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT);

	if (metrics.invocationCount > 2)
		tqManager.startTiming(cmdBuf, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	estimator(cmdBuf, localTarget, args);
	if (metrics.invocationCount > 2)
		tqManager.endTiming(cmdBuf, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	getCmdAccumulate(localTarget, localAccumulationTarget, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	if (!(flags & IEC_RUN_NO_MSE))
	{
		cmdComputeMSE(cmdBuf, localAccumulationTargetLeft, localAccumulationTargetRight, mseBuffer, &mseResources);
		cmdComputeRMSE(cmdBuf, localAccumulationTargetLeft, localAccumulationTargetRight, rmseBuffer, &rmseResources);

		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		cmdCopyBufferRegion(cmdBuf, mseBuffer, mseOverTimeBuffer, 0,(metrics.invocationCount % 100000) * sizeof(float), sizeof(float));
		cmdCopyBufferRegion(cmdBuf, rmseBuffer, rmseOverTimeBuffer, 0,(metrics.invocationCount % 100000) * sizeof(float), sizeof(float));
		//float test = 1.0 * metrics.invocationCount;
		//cmdWriteCopy(cmdBuf, mseBuffer, &test, sizeof(float));
		//cmdCopyBufferRegion(cmdBuf, mseBuffer, mseOverTimeBuffer, 0,(metrics.invocationCount % 100000) * sizeof(float), sizeof(float));
		mse.push_back(getMSE());
	}

	if (tqManager.updateTimings())
	{
		metrics.addTiming(tqManager.timings[0]);
	}
	metrics.invocationCount++;

	if (timings && metrics.hasTiming())
	{
		*timings = metrics.getAvgTiming();
	}
}


template <class EstimatorArgs>
inline void ImageEstimatorComparator::cmdRunEqualTime(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator,
                                                      EstimatorArgs argsLeft, EstimatorArgs argsRight,
                                                      float *timigsLeft, float *timigsRight, uint32_t flags)
{
	if (metricsLeft.getTotalRuntime() < metricsRight.getTotalRuntime())
	{
		cmdRun<EstimatorArgs>(cmdBuf, estimator, argsLeft, IEC_TARGET_LEFT, timigsLeft, flags);
	}
	else if (metricsLeft.getTotalRuntime() > metricsRight.getTotalRuntime())
	{
		cmdRun<EstimatorArgs>(cmdBuf, estimator, argsRight, IEC_TARGET_RIGHT, timigsRight, flags);
	}
	else
	{
		cmdRun<EstimatorArgs>(cmdBuf, estimator, argsLeft, IEC_TARGET_LEFT, timigsLeft, flags);
		cmdRun<EstimatorArgs>(cmdBuf, estimator, argsRight, IEC_TARGET_RIGHT, timigsRight, flags);
	}
}

template <class EstimatorArgs>
inline void ImageEstimatorComparator::cmdRunEqualRate(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator,
                                                      EstimatorArgs argsLeft, EstimatorArgs argsRight,
                                                      float *timigsLeft, float *timigsRight, uint32_t flags)
{
	cmdRun<EstimatorArgs>(cmdBuf, estimator, argsLeft, IEC_TARGET_LEFT, timigsLeft, flags);
	cmdRun<EstimatorArgs>(cmdBuf, estimator, argsRight, IEC_TARGET_RIGHT, timigsRight, flags);
}        // namespace vka
}        // namespace vka