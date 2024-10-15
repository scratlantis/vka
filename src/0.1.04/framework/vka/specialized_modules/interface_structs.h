#pragma once
#include <vka/advanced_state/AdvancedState.h>
#include <vka/advanced_utility/complex_commands.h>
#include <vka/pbr/universal/UScene.h>
using namespace glm;
namespace vka
{
namespace default_scene
{
	#include <vka/shaders/lib/default_scene/interface_structs.glsl>
	#include <vka/shaders/lib/interface_structs.glsl>
}
using namespace default_scene;
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

template <>
struct instance_type<GLSLInstance>
{
	ComputeCmd get_cmd_write_tlas_instance(Buffer instanceBuffer, Buffer tlasInstanceBuffer, uint32_t instanceCount)
	{
		ComputeCmd cmd(instanceCount, cVkaShaderPath + "default_scene/instance_to_tlas_instance.comp", {{"INPUT_SIZE", instanceCount}});
		cmd.pushDescriptor(instanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		cmd.pushDescriptor(tlasInstanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		return cmd;
	}
};

}        // namespace pbr



}        // namespace vka

