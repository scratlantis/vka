#include "config.h"
#include "shader_interface.h"
#include "ui.h"
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gAppShaderRoot   = std::string(APP_SRC_DIR) + "/shaders/";
using namespace glm;


#include <shaders/interface_structs.glsl>

GVar gvar_particle_render_size{"Particle Size", 1.f, GVAR_FLOAT_RANGE, GUI_CAT_RENDER, {0.1f, 10.f}};

extern GVar gvar_menu;

int main()
{
	//// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700, true);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();
	//// Init swapchain attachments
	Image img_shaded              = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_GENERAL, viewDimensions.width, viewDimensions.height);
	gState.updateSwapchainAttachments();
	//// Init other stuff
	Buffer particleBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	//// Load stuff:
	//CmdBuffer cmdBuf = createCmdBuffer(gState.frame->stack);
	//executeImmediat(cmdBuf);

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
		bool reset = gState.io.keyPressedEvent[GLFW_KEY_R]
			|| gState.io.keyPressedEvent[GLFW_KEY_Q]
			|| frameCount == 0;

		std::vector<bool> settingsChanged = buildGui();

		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		if(reset || guiCatChanged(GUI_CAT_PARTICLES, settingsChanged))
		{
			cmdGenParticles(cmdBuf, particleBuffer);
		}

		// Fill swapchain with background color
		Image     swapchainImg = getSwapchainImage();
		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.25, 0.25, 0.3, 1.0)).exec(cmdBuf);

		// Render
		{
			getCmdFill(img_shaded, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.8, 0.25, 0.3, 1.0)).exec(cmdBuf);

		}
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);


		// Composition
		{
			MapImgArgs mArgs{};
			mArgs.useScissors = true;
			mArgs.srcArea = img_shaded->getExtent2D();
			mArgs.dstArea = getScissorRect(viewDimensions);
			mArgs.dstLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


			getCmdMapImg(img_shaded, swapchainImg, mArgs).exec(cmdBuf);
			cmdRenderGui(cmdBuf, swapchainImg);
		}
		swapBuffers({cmdBuf});
		frameCount++;
	}
	gState.destroy();
}