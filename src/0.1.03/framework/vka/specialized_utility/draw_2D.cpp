#include "draw_2D.h"
#include <vka/advanced_utility/complex_commands.h>
namespace vka
{

DrawCmd getCmdFill(Image dst, glm::vec4 color)
{
	return getCmdFill(dst, dst->getLayout(), color);
}
DrawCmd getCmdFill(Image dst, VkImageLayout targetLayout, glm::vec4 color)
{
	VkRect2D_OP area = {0, 0, dst->getExtent2D().width, dst->getExtent2D().height};
	return getCmdDrawRect(dst, targetLayout, color, area);
}

DrawCmd getCmdDrawRect(Image dst, VkImageLayout targetLayout, glm::vec4 color, VkRect2D_OP area)
{
	DrawCmd drawCmd = DrawCmd();
	drawCmd.setGeometry(DrawSurface::screenFillingTriangle());
	drawCmd.pushColorAttachment(dst, targetLayout);
	drawCmd.pushConstant(&color, sizeof(glm::vec4), VK_SHADER_STAGE_FRAGMENT_BIT);
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill.frag");
	return drawCmd;
}
} // namespace vka