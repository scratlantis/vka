#pragma once
#include "../Resource.h"

namespace vka
{

class ShaderNotFoundException : public std::runtime_error
{
  public:
	ShaderNotFoundException(const std::string &msg) : std::runtime_error(msg){};
};

struct ShaderArgs
{
	std::string name;
	std::string value;

	ShaderArgs() = default;
	ShaderArgs(std::string name, std::string value) : name(name), value(value){};
	ShaderArgs(std::string name, uint32_t value) :
	    name(name), value(std::to_string(value)){};


	bool operator==(const ShaderArgs &other) const
	{
		return name == other.name && value == other.value;
	}
};

class ShaderDefinition : public ResourceIdentifier
{
  public:
	ShaderDefinition() = default;
	ShaderDefinition(std::string path, std::vector<ShaderArgs> args) :
	    path(path), args(args){};
	std::string             path;
	std::vector<ShaderArgs> args;
	std::vector<std::string> libs;


	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const ShaderDefinition &other) const;
	hash_t hash() const override;

	std::string fileID() const;
	std::string fileIDShort() const;
	std::string fileName() const;
	std::string suffix() const;

	std::string preprocessedPath() const;
	static std::string preprocessedLibPath(const std::string &path);

  protected:
};

class Shader_R : public Cachable_T<VkShaderModule>
{
  private:
	void preprocess(ShaderDefinition const &def);
	void compile(ShaderDefinition const &def);
	void createModule(ShaderDefinition const &def);

  public:
	virtual void free() override;
	Shader_R(IResourceCache *pCache, ShaderDefinition const &definition);
};

static VkPipelineShaderStageCreateInfo makeShaderStageCI(const ShaderDefinition &def, VkShaderModule shaderModule)
{
	{
		VkPipelineShaderStageCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
		ci.module          = shaderModule;
		ci.pName           = "main";
		std::string suffix = def.path.substr(def.path.find_last_of(".") + 1);
		if (suffix == "vert")
		{
			ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if (suffix == "frag")
		{
			ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		else if (suffix == "geom")
		{
			ci.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		}
		else if (suffix == "comp")
		{
			ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		}
		else if (suffix == "rgen")
		{
			ci.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		}
		else if (suffix == "rmiss")
		{
			ci.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		}
		else if (suffix == "rahit")
		{
			ci.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		}
		else if (suffix == "rchit")
		{
			ci.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		}
		else
		{
			DEBUG_BREAK;
		}
		return ci;
	}
}
}        // namespace vka

DECLARE_HASH(vka::ShaderDefinition, hash)