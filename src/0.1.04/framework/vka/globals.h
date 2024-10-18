#pragma once
//#include <vka/core/core_state/CoreState.h>
#include <vka/advanced_state/AdvancedState.h>

extern vka::AdvancedState    gState;
extern const std::string gShaderOutputDir;
extern const std::string        gAppShaderRoot;
extern std::vector<std::string> gShaderPrintfOutput;
extern std::unordered_map<std::string, std::string> gShaderPrintfOutputMap;