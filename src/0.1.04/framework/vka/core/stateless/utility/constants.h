#pragma once
#include <map>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace vka
{

struct VulkanFormatInfo
{
	uint32_t size;
	uint32_t channel_count;
};

extern const std::map<VkFormat, VulkanFormatInfo> cVkFormatTable;
extern const VkExtent2D cResolution4k;
extern const VkExtent2D cResolution2k;
extern const std::string cVkaShaderRoot;
extern const std::string cVkaShaderPath;
extern const std::string cVkaShaderLibPath;
extern const std::string cVkaShaderModulePath;
extern const std::vector<glm::vec3> cCubeVertecies;
extern const std::vector<uint32_t> cCubeTriangleIndices;
extern const std::vector<uint32_t> cCubeLineIndices;


}        // namespace vka