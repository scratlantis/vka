#pragma once
#include <vka/advanced_utility/complex_commands.h>
namespace vka
{
DrawCmd getCmdFill(Image dst, glm::vec4 color);
DrawCmd getCmdFill(Image dst, VkImageLayout targetLayout, glm::vec4 color);
DrawCmd getCmdDrawRect(Image dst, VkImageLayout targetLayout, glm::vec4 color, VkRect2D_OP area);
}