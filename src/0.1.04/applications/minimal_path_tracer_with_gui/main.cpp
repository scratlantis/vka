#include "config.h"
#include "shader_interface.h"
#include "ui.h"
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gAppShaderRoot   = std::string(APP_SRC_DIR) + "/shaders";
using namespace glm;

GVar perlinFrequency{"Perlin frequency", 4.f, GVAR_FLOAT_RANGE, GUI_CAT_NOISE, {1.f, 10.f}};

int main()
{
	//// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();
	//// Init swapchain attachments
	Image img_pt              = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, viewDimensions.width, viewDimensions.height);
	Image img_pt_accumulation = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, viewDimensions.width, viewDimensions.height);
	gState.updateSwapchainAttachments();
	//// Init other stuff
	FixedCamera                                           cam          = FixedCamera(DefaultFixedCameraState());
	USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial, GLSLInstance>();
	USceneData scene;
	//// Load stuff:
	CmdBuffer cmdBuf = createCmdBuffer(gState.frame->stack);
	//// Load Geometry
	sceneBuilder.loadEnvMap("/envmap/2k/autumn_field_2k.hdr", glm::uvec2(64, 64));
#ifdef RAY_TRACING_SUPPORT
	GLSLInstance instance{};
	instance.cullMask = 0xFF;
	instance.mat      = getMatrix(vec3(0, 0.2, -0.3), vec3(0.0, 180.0, 0.0), 0.1);
	sceneBuilder.addModel(cmdBuf, "cornell_box/cornell_box.obj", &instance, 1);
	scene = sceneBuilder.create(cmdBuf, gState.heap);
	scene.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, gState.heap));
#endif
	//// Load Medium
	PerlinNoiseArgs perlinArgs{};
	perlinArgs.scale         = 1000.0;
	perlinArgs.min           = 0.0;
	perlinArgs.max           = 100.0;
	perlinArgs.frequency          = perlinFrequency.val.v_float;
	perlinArgs.falloffAtEdge = true;
	const uint32_t mediumExtent1D = 64;
	VkExtent3D     mediumExtent{mediumExtent1D, mediumExtent1D, mediumExtent1D};
	Image          medium = createImage(gState.heap, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, mediumExtent);
	cmdTransitionLayout(cmdBuf, medium, VK_IMAGE_LAYOUT_GENERAL);
	getCmdPerlinNoise(medium, perlinArgs).exec(cmdBuf);
	GLSLMediumInstance mediumInstance{};
	mediumInstance.mat = getMatrix(vec3(-0.2, -0.2, -0.2), vec3(0, 0, 0), 0.4);
	mediumInstance.invMat = glm::inverse(mediumInstance.mat);
	mediumInstance.albedo  = vec3(1.0, 1.0, 1.0);
	Buffer mediumInstanceBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	cmdWriteCopy(cmdBuf, mediumInstanceBuffer, &mediumInstance, sizeof(GLSLMediumInstance));
	//// Clear accumulation target
	cmdFill(cmdBuf, img_pt_accumulation, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0, 0.0, 1.0, 1.0));
	executeImmediat(cmdBuf);
	// Main Loop
	uint32_t frameCount = 0;
	while (!gState.io.shouldTerminate())
	{
		bool shaderRecompiled = false;
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
			gState.io.buildShaderLib();
			shaderRecompiled = true;
		}
		bool viewHasChanged = cam.keyControl(0.016);
		viewHasChanged = (gState.io.mouse.rightPressed && cam.mouseControl(0.016)) || viewHasChanged;
		viewHasChanged      = viewHasChanged || gState.io.swapchainRecreated();

		std::vector<bool> settingsChanged = buildGui();
		bool              anySettingsChanged = orOp(settingsChanged);

		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		// Fill swapchain with background color
		Image     swapchainImg = getSwapchainImage();
		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.25, 0.25, 0.3, 1.0)).exec(cmdBuf);
		// Reset accumulation
		if (viewHasChanged || anySettingsChanged || shaderRecompiled)
		{
			cmdFill(cmdBuf, img_pt_accumulation, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0,0.0,0.0,0.0));
		}
		// Regenerate noise
		if (settingsChanged[GUI_CAT_NOISE] || shaderRecompiled)
		{
			perlinArgs.frequency = perlinFrequency.val.v_float;
			getCmdPerlinNoise(medium, perlinArgs).exec(cmdBuf);
		}
		// Path tracing
		{
			CameraCI camCI{};
			camCI.pos      = cam.getPosition();
			camCI.frontDir = cam.getViewDirection();
			camCI.upDir    = cam.getViewUpDirection();
			camCI.seed     = frameCount;
			camCI.extent = img_pt->getExtent2D();
			camCI.yFovDeg = 60.0;
			camCI.zNear  = 0.1;
			camCI.zFar = 100.0;

			TraceArgs traceArgs{};
			traceArgs.sampleCount          = 1;
			traceArgs.maxDepth             = 5;
			traceArgs.rayMarchStepSize     = 0.1;
			traceArgs.cameraCI             = camCI;
			traceArgs.sceneData            = scene;
			traceArgs.mediumInstanceBuffer = mediumInstanceBuffer;
			traceArgs.mediumTexture        = medium;

			cmdTrace(cmdBuf, img_pt, traceArgs);
		}
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		// Accumulation
		{
			getCmdAccumulate(img_pt, img_pt_accumulation, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
			getCmdNormalize(img_pt_accumulation, swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VkRect2D_OP(img_pt_accumulation->getExtent2D()), getScissorRect(viewDimensions)).exec(cmdBuf);
		}
		// Gui
		{
			cmdRenderGui(cmdBuf, swapchainImg);
		}
		swapBuffers({cmdBuf});
		frameCount++;
	}
	gState.destroy();
}