#pragma once
//#include <vka/core/core_state/CoreState.h>
#include <vka/advanced_state/AdvancedState.h>
#include <vka/advanced_state/vr/VrState.h>

//#ifndef VR_INTEGRATION
//	extern vka::AdvancedState    gState;
//#else
//	extern vka::VrState         gState;
//#endif        // !VR_INTEGRATION

extern vka::VrState             gState;
extern const std::string gShaderOutputDir;
extern const std::string        gAppShaderRoot;
extern std::vector<std::string> gShaderPrintfOutput;
extern std::unordered_map<std::string, std::string> gShaderPrintfOutputMap;