#include "config.h"

struct TraceArgs
{
	uint32_t   sampleCount;
	uint32_t   maxDepth;
	float      rayMarchStepSize;
	CameraCI   cameraCI;
	USceneData sceneData;
	Buffer     mediumInstanceBuffer;
	Image      mediumTexture;
};



void cmdTrace(CmdBuffer cmdBuf, Image target, TraceArgs args);