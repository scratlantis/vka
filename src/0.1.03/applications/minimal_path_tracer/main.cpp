#include "config.h"
#include "utility.h"
#include "shader_interface.h"

// State and output dir must be defined in the main file
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;

// Gui variable
static GVar                gvar_model = {"Model", 0, GVAR_ENUM, 0, {"Cornell Box"}};
static std::vector<GVar *> gVars =
    {
        // clang-format off
        &gvar_model
        // clang-format on
};

static ShaderConst sConst{};


ModelInfo cornellBox = {"cornell_box/cornell_box.obj", vec3(0,0.2,-0.3), 0.1, 180.0};
std::vector<ModelInfo> models = {cornellBox};

int main()
{
	// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();

	// May be added to the global state in the future
	HdrImagePdfCache pdfCache = HdrImagePdfCache(gState.heap);

	USceneBuilder<GLSLVertex, GLSLMaterial> sceneBuilder = USceneBuilder<GLSLVertex, GLSLMaterial>(&pdfCache);

	// Camera
	FixedCamera cam = FixedCamera(DefaultFixedCameraCI());

	// Persistent Resources:
	// HDR Images for path tracing
	Image img_pt = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, 0.8, 1.0);
	Image img_pt_accumulation = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, 0.8, 1.0);
	gState.updateSwapchainAttachments();
	// Uniform Buffers
	sConst.alloc();

	uint32_t modelIndexLastFrame = 0;
	USceneData scene;
	ModelInfo  model;
	// Main Loop
	for (uint cnt = 0; !gState.io.shouldTerminate(); cnt++)
	{
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
		}
		if (gState.io.keyPressedEvent[GLFW_KEY_T])
		{
			cam = FixedCamera(DefaultFixedCameraCI());
		}
		if (modelIndexLastFrame != gvar_model.val.v_uint || cnt == 0)
		{
			if (cnt != 0)
			{
				scene.garbageCollect();
			}

			modelIndexLastFrame = gvar_model.val.v_uint;
			model = models[modelIndexLastFrame];
			CmdBuffer cmdBuf = createCmdBuffer(gState.heap);
			sceneBuilder.reset();
			sceneBuilder.addModel(cmdBuf, model.path, model.getObjToWorldMatrix());
			scene = sceneBuilder.create(cmdBuf, gState.heap, SCENE_LOAD_FLAG_ALLOW_RASTERIZATION);
			scene.build(cmdBuf, sceneBuilder.uploadInstanceData(cmdBuf, gState.heap));
			executeImmediat(cmdBuf);
		}
		
		bool viewHasChanged = cam.keyControl(0.016);
		if (gState.io.mouse.rightPressed)
		{
			viewHasChanged = viewHasChanged || cam.mouseControl(0.016);
		}
		
		CmdBuffer  cmdBuf       = createCmdBuffer(gState.frame->stack);
		Image      swapchainImg = getSwapchainImage();
		if (viewHasChanged)
		{
			cmdFill(cmdBuf, img_pt_accumulation, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.0));
		}

		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vec4(0.2, 0.2, 0.2, 1.0)).exec(cmdBuf);
		// Path tracing
		if (!gState.io.mouse.middlePressed)
		{
			// Config general parameters
			ComputeCmd computeCmd = ComputeCmd(img_pt->getExtent2D(), shaderPath + "path_tracing/pt.comp", {{"FORMAT1", getGLSLFormat(img_pt->getFormat())}});
			sConst.write(cmdBuf, computeCmd, img_pt->getExtent2D(), cam, cnt, gVars);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

			// Bind Constants
			bind_block_3(computeCmd, sConst);

			// Bind Target
			computeCmd.pushDescriptor(img_pt, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

			// Bind Scene
			bind_block_10(computeCmd, scene);

			computeCmd.exec(cmdBuf);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			getCmdAccumulate(img_pt, img_pt_accumulation, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
			getCmdNormalize(img_pt_accumulation, swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			                VkRect2D_OP(img_pt_accumulation->getExtent2D()), getScissorRect(0.2f, 0.f, 0.8f, 1.0f))
			    .exec(cmdBuf);
		}
		// Rasterization for debugging
		else
		{
			img_pt->setClearValue(ClearValue(0.0f, 0.0f, 0.0f, 1.0f));
			cmdShowTriangles<GLSLVertex>(cmdBuf, gState.frame->stack, img_pt, scene.vertexBuffer, scene.indexBuffer, &cam, model.getObjToWorldMatrix());
			getCmdAdvancedCopy(img_pt, swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			                VkRect2D_OP(img_pt->getExtent2D()), getScissorRect(0.2f, 0.f, 0.8f, 1.0f)).exec(cmdBuf);
		}
		// Add gui
		{
			gvar_gui::buildGui(gVars, {"Catergory 1"}, getScissorRect(0.f,0.f,0.2,1.0));
			shader_console_gui::buildGui(getScissorRect(0.2f, 0.f, 0.8f, 1.0f));
			cmdRenderGui(cmdBuf, swapchainImg);
		}
		swapBuffers({cmdBuf});
	}
	gState.destroy();
}