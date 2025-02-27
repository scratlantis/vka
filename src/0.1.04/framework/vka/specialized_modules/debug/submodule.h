#pragma once
#include "interface_structs.h"
#include <vka/advanced_utility/complex_commands.h>
#include <vka/pbr/universal/UScene.h>

namespace vka
{
namespace shader_debug
{
void bindInvocationSelection(ComputeCmd &cmd, Buffer selectionBuf);
void cmdSelectInvocation(CmdBuffer cmdBuf, Buffer selectionBuf, glm::vec3 selection);
void cmdSelectInvocation(CmdBuffer cmdBuf, Buffer selectionBuf, glm::vec2 selection);
void bindPtShaderState(ComputeCmd &cmd, Buffer indirectBounceBuf, Buffer directRayBuf, Buffer stateBuf);
void cmdResetPtShaderState(CmdBuffer cmdBuf, Buffer indirectBounceBuf, Buffer directRayBuf, Buffer stateBuf,
                           uint32_t maxBounces, uint32_t maxDirectRaysPerBounce);
}
}        // namespace vka
