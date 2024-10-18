#pragma once
#include "interface_structs.h"
#include <vka/advanced_utility/complex_commands.h>
#include <vka/pbr/universal/UScene.h>

namespace vka
{
namespace default_scene
{
struct CameraCI
{
	vec3       pos;
	vec3       frontDir;
	vec3       upDir;
	uint       seed;
	VkExtent2D extent;
	float      yFovDeg;
	float      zNear;
	float      zFar;
};
void bindCamera(ComputeCmd &cmd, Buffer camBuf, Buffer camInstBuf);
void cmdUpdateCamera(CmdBuffer cmdBuf, Buffer camBuf, Buffer camInstBuf, CameraCI ci);
void bindScene(ComputeCmd &cmd, const pbr::USceneData *pScene);
void bindMockScene(ComputeCmd &cmd);
void bindScalarField(ComputeCmd &cmd, Image scalarField, float rayMarchStepSize);
}        // namespace default_scene
}        // namespace vka