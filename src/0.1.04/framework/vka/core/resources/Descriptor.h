#pragma once
#include "unique/Buffer.h"
#include "unique/Image.h"
#include "unique/AccelerationStructure.h"
#include "cachable/Sampler.h"
namespace vka
{

class Descriptor
{
  public:
	Descriptor(){};
	Descriptor(const Buffer_R *buffer, VkDescriptorType type, VkShaderStageFlags shaderStage);
	Descriptor(Image_R *image, VkDescriptorType type, VkShaderStageFlags shaderStage);
	Descriptor(const SamplerDefinition samplerDef, VkShaderStageFlags shaderStage);
	Descriptor(const AccelerationStructure_R *accelerationStructure, VkShaderStageFlags shaderStage);

	Descriptor(std::vector<const Buffer_R *> buffers, VkDescriptorType type, VkShaderStageFlags shaderStage);
	Descriptor(std::vector<Image_R *> images, VkDescriptorType type, VkShaderStageFlags shaderStage);
	Descriptor(std::vector <SamplerDefinition> samplersDefs, VkShaderStageFlags shaderStage);

	Descriptor(const SamplerDefinition samplerDef, Image_R *image, VkShaderStageFlags shaderStage);

	bool getLayoutTransforms(std::vector<Image_R *> &images, std::vector<VkImageLayout> &layouts) const;
	void writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos, VkWriteDescriptorSetAccelerationStructureKHR *&pAccelerationStructureWrite) const;
	void countStructures(uint32_t &bufferInfoCount, uint32_t &imageInfoCount, uint32_t &accelerationStructureWriteCount) const;
	~Descriptor(){};

  private:
	VkDescriptorType              type;
	VkShaderStageFlags            stage;
	uint32_t                      count;
	VkAccelerationStructureKHR    as;
	std::vector<const Buffer_R *> buffers;
	std::vector<Image_R *>        images;
	std::vector<VkSampler>        samplers;

};
}        // namespace vka