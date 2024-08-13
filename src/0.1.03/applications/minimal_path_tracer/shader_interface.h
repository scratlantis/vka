#pragma once
#include <vka/vka.h>

typedef uint32_t  uint;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat4 mat4;
#include "shaders/interface.glsl"

static GLSLFrame defaultFrame(VkExtent2D extent, uint32_t frameIdx)
{
	GLSLFrame frame;
	frame.idx           = frameIdx;
	frame.width         = extent.width;
	frame.height        = extent.height;
	frame.projection    = glm::perspective(glm::radians(60.0f), (float) extent.width / (float) extent.height, 0.1f, 500.0f);
	frame.invProjection = glm::inverse(frame.projection);
	return frame;
}

static GLSLView cameraView(FixedCamera cam)
{
	GLSLView view;
	view.mat    = cam.getViewMatrix();
	view.invMat = glm::inverse(view.mat);
	view.pos    = vec4(cam.getPosition(), 0.0);
	return view;
}

static GLSLParams guiParams(std::vector<GVar *> gv)
{
	GLSLParams params{};
	// Place to add gui options for shader
	return params;
}

class ShaderConst
{
  public:
	Buffer ubo_frame;
	Buffer ubo_view;
	Buffer ubo_params;

	void alloc()
	{
		ubo_frame  = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLFrame));
		ubo_view   = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLFrame));
		ubo_params = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLParams));
	}

	void free()
	{
		ubo_frame->garbageCollect();
		ubo_view->garbageCollect();
		ubo_params->garbageCollect();
	}

	void write(CmdBuffer cmdBuf, ComputeCmd &cmd, VkExtent2D extent, FixedCamera cam, uint32_t frameIdx, std::vector<GVar *> gVars)
	{
		GLSLFrame ptFrame = defaultFrame(extent, frameIdx);
		cmdWriteCopy(cmdBuf, ubo_frame, &ptFrame, sizeof(GLSLFrame));

		GLSLView ptView = cameraView(cam);
		cmdWriteCopy(cmdBuf, ubo_view, &ptView, sizeof(GLSLView));

		GLSLParams params = guiParams(gVars);
		cmdWriteCopy(cmdBuf, ubo_params, &params, sizeof(GLSLParams));
	}
};

// see template.glsl

void bind_block_3(ComputeCmd &cmd, const ShaderConst &sConst)
{
	cmd.pushDescriptor(sConst.ubo_frame, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.pushDescriptor(sConst.ubo_view, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.pushDescriptor(sConst.ubo_params, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}
void bind_block_10(ComputeCmd &cmd, const USceneData &uScene)
{
	cmd.pushDescriptor(uScene.vertexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(uScene.indexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(uScene.modelOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(uScene.surfaceOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(uScene.materialBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(uScene.areaLightBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(uScene.tlas);
	cmd.pushDescriptor(SamplerDefinition());
	cmd.pushDescriptor(uScene.textures, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	cmd.pushDescriptor(uScene.envMap, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

namespace vka
{
template <>
struct vertex_type<GLSLVertex>
{
	VertexDataLayout data_layout()
	{
		VertexDataLayout layout{};
		layout.formats =
		    {
		        VK_FORMAT_R32G32B32_SFLOAT,
		        VK_FORMAT_R32G32B32_SFLOAT,
		        VK_FORMAT_R32G32_SFLOAT,
		    };
		layout.offsets =
		    {
		        offsetof(GLSLVertex, pos),
		        offsetof(GLSLVertex, normal),
		        offsetof(GLSLVertex, uv)};
		layout.stride = sizeof(GLSLVertex);
		return layout;
	}
	void load_obj(Buffer vertexBuffer, const std::vector<ObjVertex> &vertexList)
	{
		vertexBuffer->changeSize(vertexList.size() * sizeof(GLSLVertex));
		vertexBuffer->changeMemoryType(VMA_MEMORY_USAGE_CPU_ONLY);
		vertexBuffer->recreate();
		GLSLVertex *vertexData = static_cast<GLSLVertex *>(vertexBuffer->map());
		for (size_t i = 0; i < vertexList.size(); i++)
		{
			vertexData[i].pos    = vertexList[i].v;
			vertexData[i].normal = vertexList[i].vn;
			vertexData[i].uv     = vertexList[i].vt;
		}
	}
};
}        // namespace vka

namespace vka
{
namespace pbr
{
template <>
struct material_type<GLSLMaterial>
{
	GLSLMaterial load_mtl(WavefrontMaterial mtl, std::unordered_map<std::string, uint32_t> &textureIndexMap)
	{
		GLSLMaterial material{};
		material.albedo    = mtl.diffuse;
		material.specular  = mtl.specular;
		material.roughness = mtl.roughness;
		material.emission  = mtl.emission;
		material.f0        = 0.1;        // How do we get this?
		return material;
	}
};
}        // namespace pbr
}        // namespace vka