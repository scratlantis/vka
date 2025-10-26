#include "config.h"
#include "shader_interface.h"
#include "ui.h"
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;
const std::string gAppShaderRoot   = std::string(APP_SRC_DIR) + "/shaders/";
using namespace glm;
using namespace vka::physics;


#include <shaders/interface_structs.glsl>
#include <bindings/interface_structs.h>


extern GVar gvar_menu;
extern GVar gvar_particle_size;
extern GVar gvar_particle_generation_count;

GVar gvar_simulation_step_count{"Simulation Steps Per Frame", 1U, GVAR_UINT_RANGE, GUI_CAT_GENERAL, {1U, 4U}};
GVar gvar_display_frame_time{"Frame Time: %.4f ms", 1.f, GVAR_DISPLAY_FLOAT, GUI_CAT_GENERAL};

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
	Buffer particleMemory = createBuffer(gState.heap, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	// Preallocate particle buffer
	particleMemory->changeSize(gvar_particle_generation_count.set.range.max.v_uint * sizeof(GLSLParticle));
	particleMemory->recreate();

	SimulationResources simResources{};
	simResources.init(gState.heap, gvar_particle_generation_count.set.range.max.v_uint);
	//simResources.init(gState.heap);


#if 0
	// Load stuff:
	const uint32_t test_size = 10000;
	uint32_t seed = 12345;
	std::hash<uint32_t> h;
	std::vector<uint32_t> randomNumbers(test_size);
	std::vector<uint32_t> sortedRandomNumbers(test_size);
	std::vector<uint32_t> permutations(test_size);
	for (size_t i = 0; i < randomNumbers.size(); i++)
	{
		randomNumbers[i] = seed;
		seed = h(seed);
	}
	Buffer randomBuf = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	Buffer permBuf = createBuffer(gState.heap, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	CmdBuffer cmdBuf = createCmdBuffer(gState.heap);
	cmdWriteCopy(cmdBuf, randomBuf, randomNumbers.data(), randomNumbers.size() * sizeof(uint32_t));
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	cmdRadixSortPermutation(cmdBuf, randomBuf, permBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
	Buffer localBuf = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY, randomNumbers.size() * sizeof(uint32_t));
	Buffer localPermBuf = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY, randomNumbers.size() * sizeof(uint32_t));
	cmdCopyBuffer(cmdBuf, randomBuf, localBuf);
	cmdCopyBuffer(cmdBuf, permBuf, localPermBuf);
	executeImmediat(cmdBuf);
	void* pData = localBuf->map();
	memcpy(sortedRandomNumbers.data(), pData, sortedRandomNumbers.size() * sizeof(uint32_t));
	pData = localPermBuf->map();
	memcpy(permutations.data(), pData, permutations.size() * sizeof(uint32_t));

	for (size_t i = 0; i < randomNumbers.size(); i++)
	{
		//std::cout << randomNumbers[i] << ", ";
	}
	std::cout << std::endl;

	for (size_t i = 0; i < sortedRandomNumbers.size(); i++)
	{
		if (i > 0 && sortedRandomNumbers[i] < sortedRandomNumbers[i - 1])
		{
			__debugbreak();
		}
		if(randomNumbers[permutations[i]] != sortedRandomNumbers[i])
		{
			__debugbreak();
		}
		//std::cout << "(" << sortedRandomNumbers[i] << ", " << permutations[i] << "), ";
	}
	std::cout << std::endl;
#endif
	// Main Loop
	uint32_t frameCount = 0;
	while (!gState.io.shouldTerminate())
	{
		gvar_display_frame_time.val.v_float = static_cast<float>(gState.frameTime) * 0.001f;

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

		const Buffer    particleBuffer = particleMemory->getSubBuffer({0, gvar_particle_generation_count.val.v_uint * sizeof(GLSLParticle)});
		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		if(reset || guiCatChanged(GUI_CAT_PARTICLE_GEN, settingsChanged))
		{
			cmdGenParticles(cmdBuf, particleBuffer);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

		}

		// Simulate
		for (uint32_t step = 0; step < gvar_simulation_step_count.val.v_uint; step++)
		{
			cmdSimulateParticles<GLSLParticle>(cmdBuf, particleBuffer, simResources, gState.frameTime * (1.f / static_cast<float>(gvar_simulation_step_count.val.v_uint)));
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
		}

		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

		// Fill swapchain with background color
		Image     swapchainImg = getSwapchainImage();
		getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vec4(0.25, 0.25, 0.3, 1.0)).exec(cmdBuf);

		// Render
		{
			img_shaded->setClearValue(ClearValue::black());
			cmdRenderParticles(cmdBuf, img_shaded, particleBuffer, simResources.densityBuffer);
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
	GVar::storeAll("particle_sim_last_session.json");
}