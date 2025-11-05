#include "config.h"
#include "shader_interface.h"
#include "ui.h"
#include "resources.h"

VrState     gState;
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
extern GVar gvar_camera_reset;


GVar gvar_application_mode{"Application Mode", 0U, GVAR_ENUM, GUI_CAT_GENERAL, std::vector<std::string>{"2D", "3D"}};
GVar gvar_display_frame_time{"Frame Time: %.4f ms", 1.f, GVAR_DISPLAY_FLOAT, GUI_CAT_GENERAL};
GVar gvar_sliding_average_coef {"Sliding Average Coefficient", 1.0f, GVAR_FLOAT_RANGE, GUI_CAT_GENERAL, {0.5f, 1.f}};

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
	gState.enableVr();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();
	//// Init swapchain attachments
	Image img_shaded = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
	                                             VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
	                                                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	                                             VK_IMAGE_LAYOUT_GENERAL,
	                                             viewDimensions.width, viewDimensions.height);

	Image last_img_shaded = createSwapchainAttachment(VK_FORMAT_R32G32B32A32_SFLOAT,
	                                                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
	                                                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	                                                  VK_IMAGE_LAYOUT_GENERAL,
	                                                  viewDimensions.width, viewDimensions.height);
	gState.updateSwapchainAttachments();
	//// Init other stuff
	uint64_t curFrameCount = 0;
	ApplicationMode curAppMode = static_cast<ApplicationMode>(gvar_application_mode.val.v_uint);
	ParticleResources particleRes{};
	FixedCamera       cam = loadCamState();


	// Main Loop
	while (!gState.io.shouldTerminate())
	{
		gvar_display_frame_time.val.v_float = static_cast<float>(gState.frameTime);

		bool shaderRecompiled = false;
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
			gState.io.buildShaderLib();
			shaderRecompiled = true;
		}
		bool reset = shaderRecompiled
			|| gState.io.keyPressedEvent[GLFW_KEY_Q]
			|| curFrameCount == 0;

		std::vector<bool> settingsChanged = buildGui();

		if (curFrameCount == 0 || curAppMode != static_cast<ApplicationMode>(gvar_application_mode.val.v_uint))
		{
			if (curAppMode == AM_2D)
				GVar::storeSelect("simulation_params_2D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});
			else if (curAppMode == AM_3D)
				GVar::storeSelect("simulation_params_3D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});

			curAppMode = static_cast<ApplicationMode>(gvar_application_mode.val.v_uint);

			if (curAppMode == AM_2D)
				GVar::loadSelect("simulation_params_2D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});
			else if (curAppMode == AM_3D)
				GVar::loadSelect("simulation_params_3D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});

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
			reset = true;
		}
		bool viewFocus = !GVar::holdsFocus() && mouseInView(viewDimensions);


		// Todo fix
		particleRes.desc.radius = gvar_particle_size.val.v_float * cParticle_size_scale;
		particleRes.descAttr.radius = gvar_particle_size.val.v_float * cParticle_size_scale;

		if (viewFocus)
		{
			if (curAppMode == AM_3D)
			{
				cam = FixedCamera(loadCamState());
				if(gState.io.mouse.rightPressed) cam.mouseControl();
				cam.keyControl();
				gvar_camera_reset.val.v_bool = gState.io.keyPressedEvent[GLFW_KEY_T];
				saveCamState(cam.getState());
			}
		}

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
		img_shaded->setClearValue(ClearValue::black());
		if (curAppMode == AM_2D)
		{
			getCmdRenderParticles2D(img_shaded, particleRes.getParticleBuf(), particleRes.simRes.densityBuffer).exec(cmdBuf);
		}
		else if (curAppMode == AM_3D)
		{
			default_scene::CameraCI   camCI{};
			camCI.pos      = cam.getPosition();
			camCI.frontDir = cam.getViewDirection();
			camCI.upDir    = cam.getViewUpDirection();
			camCI.frameIdx = curFrameCount;
			camCI.extent   = getScissorRect(viewDimensions).extent;
			camCI.yFovDeg  = 60.0;
			camCI.zNear    = 0.1;
			camCI.zFar     = 100.0;
			Buffer camBuf = nullptr, camInstBuf = nullptr;
			gState.dataCache->fetch(camBuf, "camBuf");
			gState.dataCache->fetch(camInstBuf, "camInstBuf");
			default_scene::cmdUpdateCamera(cmdBuf, camBuf, camInstBuf, camCI);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
			cmdShowBoxFrame(cmdBuf, gState.frame->stack, img_shaded, &cam, glm::mat4(1.0f), true, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), VK_IMAGE_LAYOUT_GENERAL);
			getCmdRenderParticles3D(img_shaded, particleRes.getParticleBuf(), particleRes.simRes.densityBuffer).exec(cmdBuf);
		}

		if (!reset)
		{
			//VkRect2D_OP fullArea = VkRect2D_OP(img_shaded->getExtent2D());
			//BlendOperation blendOp  = {1.f-gvar_sliding_average_coef.val.v_float, };
			//getCmdImageToImage(last_img_shaded, img_shaded, VK_IMAGE_LAYOUT_GENERAL, fullArea, fullArea,)
		}

		// VR Test
		{
			Image leftEyeImg = getVrSwapchainImage(XR_EYE_LEFT);
			Image rightEyeImg = getVrSwapchainImage(XR_EYE_RIGHT);

			cmdFill(cmdBuf, leftEyeImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(1.0, 0.0, 0.0, 1.0));
			cmdFill(cmdBuf, rightEyeImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vec4(0.0, 1.0, 0.0, 1.0));
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