#include "draw_2D.h"
#include <vka/advanced_utility/complex_commands.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vka/globals.h>
namespace vka
{

void cmdFill(CmdBuffer cmdBuf, Image dst, VkImageLayout dstLayout, glm::vec4 color)
{
	RenderPassDefinition rpDef = defaultRenderPass();
	addColorAttachment(rpDef, VK_IMAGE_LAYOUT_UNDEFINED, dstLayout, dst->getFormat(), true);
	VkRenderPass rp = gState.cache->fetch(rpDef);
	cmdStartRenderPass(cmdBuf, rp, gState.framebufferCache->fetch(rp, {dst}), VkRect2D_OP(dst->getExtent2D()), {ClearValue(color).value});
	cmdEndRenderPass(cmdBuf);
	dst->setLayout(dstLayout);
}

DrawCmd getCmdFill(Image dst, glm::vec4 color)
{
	return getCmdFill(dst, dst->getLayout(), color);
}
DrawCmd getCmdFill(Image dst, VkImageLayout targetLayout, glm::vec4 color)
{
	VkRect2D_OP area = {0, 0, dst->getExtent2D().width, dst->getExtent2D().height};
	return getCmdDrawRect(dst, targetLayout, color, area);
}

DrawCmd getCmdDrawRect(Image dst, VkImageLayout dstLayout, glm::vec4 color, VkRect2D_OP area)
{
	DrawCmd drawCmd = DrawCmd();
	drawCmd.setGeometry(DrawSurface::screenFillingTriangle());
	drawCmd.pushColorAttachment(dst, dstLayout);
	drawCmd.pushConstant(&color, sizeof(glm::vec4), VK_SHADER_STAGE_FRAGMENT_BIT);
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_color.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_color.frag");
	return drawCmd;
}

DrawCmd getCmdAdvancedCopy(Image src, Image dst, VkImageLayout dstLayout)
{
	return getCmdImageToImage(src, dst, dstLayout, VkRect2D_OP(src->getExtent2D()), VkRect2D_OP(dst->getExtent2D()), BlendOperation::write(), BlendOperation::write());
}

DrawCmd getCmdAdvancedCopy(Image src, Image dst, VkImageLayout dstLayout, VkRect2D_OP srcArea, VkRect2D_OP dstArea)
{
	return getCmdImageToImage(src, dst, dstLayout, srcArea, dstArea, BlendOperation::write(), BlendOperation::write());
}

DrawCmd getCmdImageToImage(Image src, Image dst, VkImageLayout dstLayout, VkRect2D_OP srcArea, VkRect2D_OP dstArea, BlendOperation colorBlendOp, BlendOperation alphaBlendOp)
{
	DrawCmd drawCmd = DrawCmd();
	drawCmd.setGeometry(DrawSurface::screenFillingTriangle());
	if (colorBlendOp == BlendOperation::write() && alphaBlendOp == BlendOperation::write())
	{
		drawCmd.pushColorAttachment(dst, dstLayout);
	}
	else
	{
		drawCmd.pushColorAttachment(dst, dstLayout, colorBlendOp, alphaBlendOp);
	}
	drawCmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	Rect2D<float> region = VkRect2D_OP::relRegion(dstArea, srcArea);
	drawCmd.pushConstant(&region, sizeof(Rect2D<float>), VK_SHADER_STAGE_FRAGMENT_BIT);
	drawCmd.renderArea = dstArea;
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_texture.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_texture.frag");
	return drawCmd;
}

Image cmdCreateDummyTexture(CmdBuffer cmdBuf, IResourcePool *pPool, VkExtent2D extent, VkFormat format, glm::vec4 color)
{
	Image img = createImage(pPool, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, extent);
	cmdFill(cmdBuf, img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, color);
	return img;
}

Image cmdCreateDummyTexture(CmdBuffer cmdBuf, IResourcePool *pPool, VkExtent2D extent)
{
	return cmdCreateDummyTexture(cmdBuf, pPool, extent, VK_FORMAT_R8G8B8A8_UNORM, glm::vec4(0.0));
}

DrawCmd getCmdAccumulate(Image src, Image dst, VkImageLayout dstLayout)
{
	return getCmdImageToImage(src, dst, dstLayout, src->getExtent2D(), dst->getExtent2D(), BlendOperation::add(), BlendOperation::add());
}

DrawCmd getCmdAccumulate(Image src, Image dst, VkImageLayout dstLayout, VkRect2D_OP srcArea, VkRect2D_OP dstArea)
{
	return getCmdImageToImage(src, dst, dstLayout, srcArea, dstArea, BlendOperation::add(), BlendOperation::add());
}

ComputeCmd getCmdNormalize(Image target, VkImageLayout dstLayout)
{
	ComputeCmd cmd(target->getExtent2D(), cVkaShaderPath + "normalizeImg.comp", {{"FORMAT", getGLSLFormat(target->getFormat())}});
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	glm::uvec2 extent = glm::uvec2(target->getExtent2D().width, target->getExtent2D().height);
	cmd.pushConstant(&extent, sizeof(glm::uvec2));
	return cmd;
}

DrawCmd getCmdNormalize(Image src, Image dst, VkImageLayout dstLayout, VkRect2D_OP srcArea, VkRect2D_OP dstArea)
{
	DrawCmd drawCmd = DrawCmd();
	drawCmd.setGeometry(DrawSurface::screenFillingTriangle());
	drawCmd.pushColorAttachment(dst, dstLayout);
	drawCmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	Rect2D<float> region = VkRect2D_OP::relRegion(dstArea, srcArea);
	drawCmd.pushConstant(&region, sizeof(Rect2D<float>), VK_SHADER_STAGE_FRAGMENT_BIT);
	drawCmd.renderArea = dstArea;
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "fill_texture.vert");
	addShader(drawCmd.pipelineDef, cVkaShaderPath + "normalizeImg.frag");
	return drawCmd;
}
}        // namespace vka
