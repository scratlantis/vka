#include "submodule.h"
#include <glm/gtx/string_cast.hpp>
namespace vka
{
namespace shader_debug
{
void bindInvocationSelection(ComputeCmd &cmd, Buffer selectionBuf)
{
	cmd.pushSubmodule(cVkaShaderModulePath + "debug/select_single_invocation.glsl");
	cmd.pushDescriptor(selectionBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}
void cmdSelectInvocation(CmdBuffer cmdBuf, Buffer selectionBuf, glm::vec2 selection)
{
	cmdSelectInvocation(cmdBuf, selectionBuf, glm::vec3(selection, 0.0));
}
void cmdSelectInvocation(CmdBuffer cmdBuf, Buffer selectionBuf, glm::vec3 selection)
{
	selectionBuf->addUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	selectionBuf->changeSize(sizeof(glm::vec3));
	selectionBuf->recreate();
	cmdWriteCopy(cmdBuf, selectionBuf, &selection, sizeof(glm::vec3));
}

void bindPtShaderState(ComputeCmd &cmd, Buffer indirectBounceBuf, Buffer directRayBuf, Buffer stateBuf)
{
	cmd.pushSubmodule(cVkaShaderModulePath + "debug/pt_shader_state.glsl");
	cmd.pushDescriptor(indirectBounceBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(directRayBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(stateBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

void cmdResetPtShaderState(CmdBuffer cmdBuf, Buffer indirectBounceBuf, Buffer directRayBuf, Buffer stateBuf,
                           uint32_t maxBounces, uint32_t maxDirectRaysPerBounce)
{
	indirectBounceBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	directRayBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	stateBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	indirectBounceBuf->changeSize(maxBounces * sizeof(GLSLIndirectBounce));
	directRayBuf->changeSize(maxBounces * maxDirectRaysPerBounce * sizeof(GLSLDirectRay));
	stateBuf->changeSize(sizeof(GLSLPtState));

	indirectBounceBuf->recreate();
	directRayBuf->recreate();
	stateBuf->recreate();

	cmdZeroBuffer(cmdBuf, indirectBounceBuf);
	cmdZeroBuffer(cmdBuf, directRayBuf);
	cmdZeroBuffer(cmdBuf, stateBuf);
}
}        // namespace shader_debug
}        // namespace vka