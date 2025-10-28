#include "config.h"
#include "shader_interface.h"
#include "ui.h"
#include "resources.h"
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gAppShaderRoot   = std::string(APP_SRC_DIR) + "/shaders/";
using namespace glm;
using namespace vka::physics;


#include <shaders/interface_structs.glsl>
#include <bindings/interface_structs.h>


static const float cParticle_size_scale = 0.1f;

extern GVar gvar_menu;
extern GVar gvar_particle_size;
extern GVar gvar_particle_generation_count;
extern GVar gvar_simulation_step_count;


GVar gvar_display_frame_time{"Frame Time: %.4f ms", 1.f, GVAR_DISPLAY_FLOAT, GUI_CAT_GENERAL};
GVar gvar_application_mode{"Application Mode", 0U, GVAR_ENUM, GUI_CAT_GENERAL, std::vector<std::string>{"2D", "3D"}};
enum ApplicationMode
{
	AM_2D,
	AM_3D
};

int main()
{
	GVar::loadAll("particle_sim_last_session.json");
	//// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700, true);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();
	//// Init swapchain attachments
	Image img_shaded              = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_LAYOUT_GENERAL,
		viewDimensions.width, viewDimensions.height);
	gState.updateSwapchainAttachments();
	//// Init other stuff
	uint64_t curFrameCount = 0;
	ApplicationMode curAppMode = static_cast<ApplicationMode>(gvar_application_mode.val.v_uint);
	ParticleResources particleRes{};

	// Main Loop
	while (!gState.io.shouldTerminate())
	{
		gvar_display_frame_time.val.v_float = static_cast<float>(gState.frameTime);

		if (curFrameCount == 0 || curAppMode != static_cast<ApplicationMode>(gvar_application_mode.val.v_uint))
		{
			curAppMode = static_cast<ApplicationMode>(gvar_application_mode.val.v_uint);
			ParticleDescription particleDesc{};
			switch (curAppMode)
			{
				case AM_2D:
					particleDesc = particle_type<GLSLParticle>::get_description(gvar_particle_size.val.v_float * cParticle_size_scale);
					break;
				case AM_3D:
					particleDesc = particle_type<GLSLParticle3D>::get_description(gvar_particle_size.val.v_float * cParticle_size_scale);
					break;
			}

			particleRes.init(gvar_particle_generation_count.set.range.max.v_uint, particleDesc, gState.heap, &curFrameCount, &gvar_particle_generation_count.val.v_uint);
		}

		bool shaderRecompiled = false;
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
			gState.io.buildShaderLib();
			shaderRecompiled = true;
		}
		bool reset = gState.io.keyPressedEvent[GLFW_KEY_R]
			|| gState.io.keyPressedEvent[GLFW_KEY_Q]
			|| curFrameCount == 0;

		std::vector<bool> settingsChanged = buildGui();

		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		if(reset || guiCatChanged(GUI_CAT_PARTICLE_GEN, settingsChanged))
		{
			getCmdGenParticles(&particleRes).exec(cmdBuf);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
		}
		// Simulate
		for (uint32_t step = 0; step < gvar_simulation_step_count.val.v_uint; step++)
		{
			cmdSimulateParticles(cmdBuf, &particleRes);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
		}

		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

		// Fill swapchain with background color
		Image     swapchainImg = getSwapchainImage();
		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.25, 0.25, 0.3, 1.0)).exec(cmdBuf);

		// Render
		{
			img_shaded->setClearValue(ClearValue::black());
			getCmdRenderParticles2D(img_shaded, particleRes.getParticleBuf(), particleRes.simRes.densityBuffer);
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
		curFrameCount++;
	}
	gState.destroy();
	GVar::storeAll("particle_sim_last_session.json");
}