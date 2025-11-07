#include "config.h"
#include "shader_interface.h"
#include "ui.h"
#include "resources.h"
#include <boxer/boxer.h>

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


void load(std::string s, std::vector<int> ids = {})
{
	std::ifstream i(s);
	if (i.is_open())
	{
		i.close();
		if (ids.empty())
		{
			GVar::loadAll(s);
		}
		else
		{
			GVar::loadSelect(s, ids);
		}
	}
	else
	{
		if (ids.empty())
		{
			GVar::loadAll(std::string(CONFIG_DIR) + "/" + s);
		}
		else
		{
			GVar::loadSelect(std::string(CONFIG_DIR) + "/" + s, ids);
		}
	}
}

#define CONTROLER_INDEX_A 0
#define CONTROLER_INDEX_B 1

int main()
{
	bool doRestart = true;
	if (boxer::Selection::Yes == boxer::show("Use VR? ", "particle simulation", boxer::Buttons::YesNo))
	{
		gState.enableVr();
	}
	while (doRestart)
	{
		doRestart = false;
		load("particle_sim_last_session.json");

		//// Global State Initialization. See config.h for more details.
		DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
		IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700, true);
		GlfwWindow          window   = GlfwWindow();
		AdvancedStateConfig config   = DefaultAdvancedStateConfig();
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

		Image eyeLeft = nullptr, eyeRight = nullptr;

		if (gState.isVrEnabled())
		{
			eyeLeft = createImage(gState.heap, gState.xrHeadset.swapchainImageFormat,
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
								   gState.xrHeadset.eyeResolution);
			eyeRight = createImage(gState.heap, gState.xrHeadset.swapchainImageFormat,
				VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
									 gState.xrHeadset.eyeResolution);

			CmdBuffer cmdBuf = createCmdBuffer(gState.frame->stack);

			cmdTransitionLayout(cmdBuf, eyeLeft, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			executeImmediat(cmdBuf);
		}



		gState.updateSwapchainAttachments();
		//// Init other stuff
		uint64_t curFrameCount = 0;
		ApplicationMode curAppMode = static_cast<ApplicationMode>(gvar_application_mode.val.v_uint);
		ParticleResources particleRes{};
		FixedCamera       cam = loadCamState();


		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat[1][1]     = -1;
		// modelMat[0][0] = -1;
		modelMat = glm::scale(modelMat, vec3(0.4));
		modelMat = glm::translate(modelMat, {-2.0f, -4.0f, 2.0f});

		//modelMat = glm::mat4(1.0);
		glm::mat4 ogModelMat = glm::mat4(modelMat);

		glm::mat4 lastModelMat = modelMat;

		glm::mat4 rightControllerMat = glm::mat4(1.0f);
		bool      isGrip             = false;
		glm::mat4 changeMat = glm::mat4(1.0f);

		uint32_t toggleFlags = PARTICLE_EARTH_GRAVITY_BIT | PARTICLE_BOX_BIT;

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

			reset = reset || gState.isVrEnabled() && gState.xrControllers.getBPress(CONTROLER_INDEX_A);

			std::vector<bool> settingsChanged = buildGui();

			if (curFrameCount == 0 || curAppMode != static_cast<ApplicationMode>(gvar_application_mode.val.v_uint))
			{
				if (curFrameCount != 0)
				{
					if (curAppMode == AM_2D)
						GVar::storeSelect("simulation_params_2D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});
					else if (curAppMode == AM_3D)
						GVar::storeSelect("simulation_params_3D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});
				}
				curAppMode = static_cast<ApplicationMode>(gvar_application_mode.val.v_uint);

				if (curAppMode == AM_2D)
					load("simulation_params_2D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});
				else if (curAppMode == AM_3D)
					load("simulation_params_3D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});

				//GVar::loadSelect("simulation_params_2D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});
				//GVar::loadSelect("simulation_params_3D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});

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


			lastModelMat = changeMat * modelMat;

			if (gState.isVrEnabled())
			{
				if (gState.xrControllers.getThumbstickPress(CONTROLER_INDEX_B) && curFrameCount > 10)
				{
					gState.io.requestTerminate();
					doRestart = true;
				}
				if (gState.xrControllers.getAPress(CONTROLER_INDEX_B))
				{
					toggleFlags ^= PARTICLE_EARTH_GRAVITY_BIT;
				}
				if (gState.xrControllers.getBPress(CONTROLER_INDEX_B))
				{
					toggleFlags ^= PARTICLE_GRAVITY_BIT;
				}
				if (gState.xrControllers.getAPress(CONTROLER_INDEX_A))
				{
					toggleFlags ^= PARTICLE_BOX_BIT;
				}

				if (gState.xrControllers.getThumbstickPress(CONTROLER_INDEX_A))
				{
					modelMat = glm::mat4(ogModelMat);
				}
				if (gState.xrControllers.getTriggerPressure(CONTROLER_INDEX_A) > 0.5f)
				{
					changeMat = gState.xrControllers.getPose(CONTROLER_INDEX_A) * glm::inverse(rightControllerMat);
					isGrip    = true;
				}
				else
				{
					if (isGrip)
					{
						isGrip    = false;
						modelMat  = changeMat * modelMat;
						changeMat = glm::mat4(1.0f);
					}
					rightControllerMat = gState.xrControllers.getPose(CONTROLER_INDEX_A);
				}
			}




			CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
			if(reset || guiCatChanged(GUI_CAT_PARTICLE_GEN, settingsChanged))
			{
				getCmdGenParticles(&particleRes).exec(cmdBuf);
				cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
			}
			GLSLParticleUpdateParams params{};
			params.modelMat             = changeMat * modelMat;
			params.modelMatLastFrame    = lastModelMat;
			params.invModelMat          = glm::inverse(changeMat * modelMat);
			params.invModelMatLastFrame = glm::inverse(lastModelMat);
			params.toggleFlags          = toggleFlags;
			params.frameIdx 			= curFrameCount;


			// Simulate
			for (uint32_t step = 0; step < gvar_simulation_step_count.val.v_uint; step++)
			{
				//cmdSimulateParticles(cmdBuf, &particleRes, {changeMat * modelMat, lastModelMat, glm::inverse(changeMat * modelMat), glm::inverse(lastModelMat)});
				cmdSimulateParticles(cmdBuf, &particleRes, params, step);
				cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
			}

			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

			// Fill swapchain with background color
			Image     swapchainImg = getSwapchainImage();
			getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.25, 0.25, 0.3, 1.0)).exec(cmdBuf);


			

			// Render
			if (!gState.isVrEnabled())
			{

			img_shaded->setClearValue(ClearValue::black());
			if (curAppMode == AM_2D)
			{
				getCmdRenderParticles2D(img_shaded, particleRes.getParticleBuf(), particleRes.simRes.densityBuffer, particleRes.simRes.forceBuffer).exec(cmdBuf);
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
				getCmdRenderParticles3D(img_shaded, particleRes.getParticleBuf(), particleRes.simRes.densityBuffer, particleRes.simRes.forceBuffer).exec(cmdBuf);
			}
			}

			if (!reset)
			{
				//VkRect2D_OP fullArea = VkRect2D_OP(img_shaded->getExtent2D());
				//BlendOperation blendOp  = {1.f-gvar_sliding_average_coef.val.v_float, };
				//getCmdImageToImage(last_img_shaded, img_shaded, VK_IMAGE_LAYOUT_GENERAL, fullArea, fullArea,)
			}

			

			// VR Test
			if (gState.isVrEnabled())
			{
				


			
				std::vector<Image> eyeImages = {eyeLeft,  eyeRight};
				//std::vector<Image> eyeImages   = {rightEyeImg, leftEyeImg};
				for (size_t i = 0; i < eyeImages.size(); i++)
				{
					eyeImages[i]->setClearValue(ClearValue(0.f,0.f, 0.f, 1.f));
					cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
					//XrFovf                  fov = gState.xrHeadset.eyePoses[i].fov;
					glm::mat4               projMat = gState.xrHeadset.eyeProjectionMatrices[i];
					glm::mat4               viewMat = gState.xrHeadset.eyeViewMatrices[i];

					default_scene::CameraCI camCI{};
					camCI.useMatricies = true;
					camCI.projMat      = projMat;
					camCI.viewMat      = viewMat * changeMat * modelMat;
					camCI.frameIdx = curFrameCount;
					camCI.extent   = gState.xrHeadset.eyeResolution;
					Buffer camBuf = nullptr, camInstBuf = nullptr;
					gState.dataCache->fetch(camBuf, "camBuf");
					gState.dataCache->fetch(camInstBuf, "camInstBuf");
					default_scene::cmdUpdateCamera(cmdBuf, camBuf, camInstBuf, camCI);
					cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
					//cmdShowBoxFrame(cmdBuf, gState.frame->stack, eyeImages[i], &cam, glm::mat4(1.0f), true, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), VK_IMAGE_LAYOUT_GENERAL);

					if ((toggleFlags & PARTICLE_BOX_BIT))
					{
						cmdShowBoxFrame(cmdBuf, gState.frame->stack, eyeImages[i], nullptr, projMat * viewMat * changeMat * modelMat, true, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), VK_IMAGE_LAYOUT_GENERAL);
					}
					else
					{
						cmdShowBoxFrame(cmdBuf, gState.frame->stack, eyeImages[i], nullptr, projMat * viewMat * changeMat * modelMat, true, glm::vec4(0.0f, 0.0f, 0.5f, 1.0f), VK_IMAGE_LAYOUT_GENERAL);
						//gState.depthBufferCache->fetch(eyeImages[i]->getExtent2D())->setClearValue(ClearValue(1.f, 0U));
					}
					getCmdRenderParticles3D(eyeImages[i], particleRes.getParticleBuf(), particleRes.simRes.densityBuffer, particleRes.simRes.forceBuffer).exec(cmdBuf);
				}
			}

		

			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);


			// Composition
			{
				MapImgArgs mArgs{};
				mArgs.useScissors = true;
				mArgs.dstArea = getScissorRect(viewDimensions);
				mArgs.dstLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


				if (gState.isVrEnabled())
				{
					Image leftEyeImg  = getVrSwapchainImage(XR_EYE_LEFT);
					Image rightEyeImg = getVrSwapchainImage(XR_EYE_RIGHT);
					MapImgArgs mArgsVr{};
					mArgsVr.dstLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

					getCmdMapImg(eyeLeft, leftEyeImg, mArgsVr).exec(cmdBuf);
					getCmdMapImg(eyeRight, rightEyeImg, mArgsVr).exec(cmdBuf);

					// Mirror view
					mArgs.srcArea = eyeLeft->getExtent2D();
					getCmdMapImg(eyeLeft, swapchainImg, mArgs).exec(cmdBuf);
				}
				else
				{
					mArgs.srcArea = img_shaded->getExtent2D();
					getCmdMapImg(img_shaded, swapchainImg, mArgs).exec(cmdBuf);
				}

				cmdRenderGui(cmdBuf, swapchainImg);
			}
			swapBuffers({cmdBuf});
			curFrameCount++;
		}

		if (curAppMode == AM_2D)
			GVar::storeSelect("simulation_params_2D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});
		else if (curAppMode == AM_3D)
			GVar::storeSelect("simulation_params_3D.json", {GUI_CAT_PARTICLE_GEN, GUI_CAT_PARTICLE_UPDATE, GUI_CAT_RENDER_GENERAL});

		gState.destroy();
		GVar::storeAll("particle_sim_last_session.json");
	}
}