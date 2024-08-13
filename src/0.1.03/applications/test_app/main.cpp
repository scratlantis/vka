#include "config.h"
#include <vka/vka.h>
using namespace vka;
AdvancedState     gState;
const std::string gShaderOutputDir = SHADER_OUTPUT_DIR;

//Gui variable
GVar                gvar_color = {"color", {1.0, 0.0, 0.0}, GVAR_VEC3, 0};
std::vector<GVar *> gVars =
{
        &gvar_color
};

int main()
{
	// Global State Initialization. See config.h for more details.
	DeviceCI            deviceCI = DefaultDeviceCI(APP_NAME);
	IOControlerCI       ioCI     = DefaultIOControlerCI(APP_NAME, 1000, 700);
	GlfwWindow          window   = GlfwWindow();
	AdvancedStateConfig config   = DefaultAdvancedStateConfig();
	gState.init(deviceCI, ioCI, &window, config);
	enableGui();

	// Main Loop
	while (!gState.io.shouldTerminate())
	{
		if (gState.io.keyPressedEvent[GLFW_KEY_R])
		{
			clearShaderCache();
		}
		Image     swapchainImg = getSwapchainImage();
		CmdBuffer cmdBuf       = createCmdBuffer(gState.frame->stack);
		DrawCmd   drawCmd      = getCmdFill(swapchainImg, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			{gvar_color.val.v_vec3[0], gvar_color.val.v_vec3[1], gvar_color.val.v_vec3[2], 1.0});
		drawCmd.exec(cmdBuf);
		gvar_gui::buildGui(gVars, {"Catergory 1"}, getScissorRect(0.f,0.f,0.2,1.0));
		cmdRenderGui(cmdBuf, swapchainImg);
		swapBuffers({cmdBuf});
	}
	gState.destroy();
}