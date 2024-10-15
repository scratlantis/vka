#include "Shader.h"
#include <filesystem>
#include <vka/globals.h>

namespace vka
{
// Overrides start
hash_t ShaderDefinition::hash() const
{
	return std::hash<std::string>()(fileID());
}

DEFINE_EQUALS_OVERLOAD(ShaderDefinition, ResourceIdentifier)

bool ShaderDefinition::operator==(const ShaderDefinition &other) const
{
	return path == other.path && cmpVector(args, other.args) && cmpVector(libs, other.libs);
}

std::string ShaderDefinition::fileID() const
{
	std::string id = path;
	for (size_t i = 0; i < args.size(); i++)
	{
		id.append("_");
		id.append(args[i].name);
		if (args[i].value != "")
		{
			id.append("=");
		}
		id.append(args[i].value);
	}

	for (size_t i = 0; i < libs.size(); i++)
	{
		id.append("_");
		id.append(libs[i]);
	}
	return id;
}

std::string ShaderDefinition::fileIDShort() const
{
	std::string name   = path.substr(path.find_last_of("/") + 1);
	name               = name.substr(0, name.find_last_of("."));
	std::string suffix = path.substr(path.find_last_of(".") + 1);
	std::string id     = name + "_" + std::to_string(hash()) + "_" + suffix;
	return id;
}

std::string ShaderDefinition::fileName() const
{
	std::string name = path.substr(path.find_last_of("/") + 1);
	return name;
}

std::string ShaderDefinition::suffix() const
{
	return path.substr(path.find_last_of(".") + 1);
}

std::string ShaderDefinition::preprocessedPath() const
{
	bool insideLib = path.compare(0, cVkaShaderRoot.size(), cVkaShaderRoot) == 0;
	VKA_ASSERT(insideLib || path.compare(0, gAppShaderRoot.size(), gAppShaderRoot) == 0);

	if (insideLib)
	{
		std::string localPath = path.substr(cVkaShaderRoot.size());
		if (localPath.find_last_of("/") == std::string::npos)
		{
			localPath = "";
		}
		else
		{
			localPath = localPath.substr(0, localPath.find_last_of("/") + 1);
		}
		return gShaderOutputDir + "/preprocessed/lib/" + localPath
			+ fileIDShort() + "." + suffix();
	}
	else
	{
		std::string localPath = path.substr(gAppShaderRoot.size() + 1);
		if (localPath.find_last_of("/") == std::string::npos)
		{
			localPath = "";
		}
		else
		{
			localPath = localPath.substr(0, localPath.find_last_of("/") + 1);
		}
		return gShaderOutputDir + "/preprocessed/app/" + localPath + fileIDShort() + "." + suffix();
	}
}

std::string ShaderDefinition::preprocessedLibPath(const std::string &path)
{
	bool insideLib = path.compare(0, cVkaShaderRoot.size(), cVkaShaderRoot) == 0;
	VKA_ASSERT(insideLib || path.compare(0, gAppShaderRoot.size(), gAppShaderRoot) == 0);

	if (insideLib)
	{
		return gShaderOutputDir + "/preprocessed/lib/" + path.substr(cVkaShaderRoot.size());
	}
	else
	{
		return gShaderOutputDir + "/preprocessed/app/" + path.substr(gAppShaderRoot.size());
	}
}

void Shader_R::free()
{
	vkDestroyShaderModule(gState.device.logical, handle, nullptr);
}

Shader_R::Shader_R(IResourceCache *pCache, ShaderDefinition const &definition) :
    Cachable_T<VkShaderModule>(pCache)
{
	createModule(definition);
}

void Shader_R::preprocess(ShaderDefinition const& def)
{
	std::string shaderPrefix = "#version 460\n#extension GL_GOOGLE_include_directive : enable\n";
	uint32_t    linesAdded      = 2;
	std::string shaderPath = def.preprocessedPath();
	std::string shaderTargetDir = shaderPath.substr(0, shaderPath.find_last_of("/"));
	std::filesystem::create_directories(shaderTargetDir);

	std::string shaderCode = std::string(readTextFile(def.path).data());
	for (auto& lib : def.libs)
	{
		std::string libPath = ShaderDefinition::preprocessedLibPath(lib);
		std::string relativePath = getRelativePath(shaderTargetDir, libPath);
		std::string include = "#include \"" + relativePath + "\"\n";
		linesAdded++;
		shaderPrefix += include;
	}
	// Add newlines to make sure the add line count is a multiple of 10, makes it easier to read the debugging log line numbers
	uint32_t roundUpLineCount = 10 - (linesAdded % 10);
	for (uint32_t i = 0; i < roundUpLineCount; i++)
	{
		shaderPrefix += "\n";
	}

	shaderCode = shaderPrefix + shaderCode;
	//for (uint32_t i = shaderCode.size() - 1; i > 0; i--)
	//{
	//	char c = shaderCode[i];
	//	if (c != '}') // Todo: very hacky, but works for now
	//	{
	//		shaderCode.pop_back();
	//	}
	//	else
	//	{
	//		break;
	//	}
	//}
	writeFile(shaderPath, shaderCode);
}


void Shader_R::compile(ShaderDefinition const &def)
{
	std::stringstream shader_src_path;
	std::stringstream shader_spv_path;
	std::stringstream shader_log_path;
	std::stringstream cmdShaderCompile;
	shader_src_path << def.preprocessedPath();

	shader_spv_path << gShaderOutputDir << "/spv";
	std::filesystem::create_directories(shader_spv_path.str());
	shader_spv_path << "/" << def.fileIDShort() << ".spv";

	shader_log_path << gShaderOutputDir << "/log";
	std::filesystem::create_directories(shader_log_path.str());
	shader_log_path << "/" << def.fileIDShort() << "_log.txt";

	cmdShaderCompile << GLSLC_COMMAND;
	for (size_t i = 0; i < def.args.size(); i++)
	{
		cmdShaderCompile << " -D" << def.args[i].name << "=" << def.args[i].value;
	}
	std::string suffix = def.path.substr(def.path.find_last_of(".") + 1);
	std::string name   = def.path.substr(0, def.path.find_last_of("."));
	name               = name.substr(name.find_last_of("/") + 1);
	for (auto &c : suffix)
		c = toupper(c);
	for (auto &c : name)
		c = toupper(c);
	cmdShaderCompile << " -D" << name << "_" << suffix;

	cmdShaderCompile << " -o " << shader_spv_path.str() << " " << shader_src_path.str()
	                 << " 2> " << shader_log_path.str();
	system(cmdShaderCompile.str().c_str());
}

void Shader_R::createModule(ShaderDefinition const &def)
{
	printVka(("Loading shader: " + def.fileName()).c_str());
	std::vector<char> shader_log;
	preprocess(def);
	compile(def);
	std::stringstream shader_log_path;
	shader_log_path << gShaderOutputDir << "/log/" << def.fileIDShort() << "_log.txt";
	std::string shader_log_path_str = shader_log_path.str();
	shader_log                      = readTextFile(shader_log_path_str);
	if (shader_log.size() > 1)
	{
		printVka("Error compiling shader '%s' : %s", def.fileIDShort().c_str(), shader_log.data());
		gState.shaderLog += "\nShader compile error:\n";
		gState.shaderLog += shader_log.data();
	}

	std::stringstream shader_spv_path;
	shader_spv_path << gShaderOutputDir << "/spv/" << def.fileIDShort() << ".spv";
	std::string shader_spv_path_str = shader_spv_path.str();
	std::vector<char> shaderCode;
	try
	{
		shaderCode = readFile(shader_spv_path_str.c_str());
	}
	catch (std::runtime_error &e)
	{
		throw ShaderNotFoundException(shader_spv_path_str + " is no valid spv!");
		return;
	}
	if (shaderCode.size() == 0)
	{
		printVka("Shader not found : %s", shader_spv_path_str.c_str());
		DEBUG_BREAK
	}
	VkShaderModuleCreateInfo shaderCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	shaderCreateInfo.codeSize = shaderCode.size();
	shaderCreateInfo.pCode    = (uint32_t *) shaderCode.data();
	VK_CHECK(vkCreateShaderModule(gState.device.logical, &shaderCreateInfo, nullptr, &handle));
}

}        // namespace vka