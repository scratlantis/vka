#include "Descriptor.h"
#include <vka/globals.h>

namespace vka
{
Descriptor::Descriptor(const Buffer_R *buffer, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(buffer != nullptr &&
	           (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
	            type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
	            type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
	            type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC));
	stage = shaderStage;
	count = 1;
	buffers = {buffer};
	this->type = type;
}
Descriptor::Descriptor(Image_R *image, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(image != nullptr &&
	           (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
	            type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE));
	stage      = shaderStage;
	count      = 1;
	images     = {image};
	this->type = type;
}
Descriptor::Descriptor(const SamplerDefinition samplerDef, VkShaderStageFlags shaderStage)
{
	stage    = shaderStage;
	count = 1;
	samplers = {gState.cache->fetch(samplerDef)};
	type = VK_DESCRIPTOR_TYPE_SAMPLER;
}
Descriptor::Descriptor(const AccelerationStructure_R *accelerationStructure, VkShaderStageFlags shaderStage)
{
	stage = shaderStage;
	count = 1;
	as    = accelerationStructure->getHandle();
	type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

}
Descriptor::Descriptor(std::vector<const Buffer_R *> buffers, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
	           type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
	           type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
	           type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
	this->buffers = buffers;
	this->type    = type;
	this->stage   = shaderStage;
	count         = buffers.size();
}
Descriptor::Descriptor(std::vector<Image_R *> images, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
	           type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	this->images = images;
	this->type   = type;
	this->stage  = shaderStage;
	count        = images.size();
}
Descriptor::Descriptor(std::vector<SamplerDefinition> samplersDefs, VkShaderStageFlags shaderStage)
{
	this->type = VK_DESCRIPTOR_TYPE_SAMPLER;
	this->stage = shaderStage;
	count = samplersDefs.size();
	for (auto &def : samplersDefs)
	{
		samplers.push_back(gState.cache->fetch(def));
	}
}
Descriptor::Descriptor(SamplerDefinition samplerDef, Image_R *image, VkShaderStageFlags shaderStage)
{
	VKA_ASSERT(image != nullptr);
	stage    = shaderStage;
	count = 1;
	images = {image};
	samplers = {gState.cache->fetch(samplerDef)};
	type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
}

bool Descriptor::getLayoutTransforms(std::vector<Image_R *> &images, std::vector<VkImageLayout> &layouts) const
{
	if (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	{
		for (auto &image : this->images)
		{
			if (image->getLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				images.push_back(image);
				layouts.push_back(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
	}
	else if (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		{
		for (auto &image : this->images)
		{
			if (image->getLayout() != VK_IMAGE_LAYOUT_GENERAL)
			{
				images.push_back(image);
				layouts.push_back(VK_IMAGE_LAYOUT_GENERAL);
			}
		}
	}
	return false;
}

void Descriptor::writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos, VkWriteDescriptorSetAccelerationStructureKHR *&pAccelerationStructureWrite) const
{
	if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
	{
		VKA_ASSERT(buffers.size() == count);
		write.descriptorCount = count;
		write.pBufferInfo   = pBufferInfo;
		write.descriptorType = type;
		for (uint32_t i = 0; i < count; i++)
		{
			pBufferInfo->buffer = buffers[i]->getHandle();
			pBufferInfo->offset = buffers[i]->getRange().offset;
			pBufferInfo->range  = buffers[i]->getRange().size;
			pBufferInfo++;
		}
		return;
	}
	if (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	{
		VKA_ASSERT(images.size() == count);
		write.descriptorCount = count;
		write.pImageInfo      = pImageInfos;
		write.descriptorType = type;
		for (uint32_t i = 0; i < count; i++)
		{
			pImageInfos->imageView   = images[i]->getViewHandle();
			pImageInfos->imageLayout = images[i]->getLayout();
			pImageInfos->sampler     = nullptr;
			pImageInfos++;
		}
		return;
	}
	if (type == VK_DESCRIPTOR_TYPE_SAMPLER)
	{
		VKA_ASSERT(samplers.size() == count);
		write.descriptorCount = count;
		write.pImageInfo      = pImageInfos;
		write.descriptorType  = type;
		for (uint32_t i = 0; i < count; i++)
		{
			pImageInfos->imageView   = VK_NULL_HANDLE;
			pImageInfos->imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			pImageInfos->sampler     = samplers[i];
			pImageInfos++;
		}
		return;
	}
	if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		{
		VKA_ASSERT(images.size() == count);
		VKA_ASSERT(samplers.size() == count);
		write.descriptorCount = count;
		write.pImageInfo      = pImageInfos;
		write.descriptorType  = type;
		for (uint32_t i = 0; i < count; i++)
		{
			pImageInfos->imageView   = images[i]->getViewHandle();
			pImageInfos->imageLayout = images[i]->getLayout();
			pImageInfos->sampler     = samplers[i];
			pImageInfos++;
		}
		return;
	    }
	    if (type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
	    {
		    VKA_ASSERT(as != nullptr);
		    write.descriptorCount                                   = count;
		    write.descriptorType                                    = type;
		    write.pNext                                             = pAccelerationStructureWrite;
		    pAccelerationStructureWrite->sType                      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		    pAccelerationStructureWrite->pNext                      = nullptr;
		    pAccelerationStructureWrite->accelerationStructureCount = 1;
		    pAccelerationStructureWrite->pAccelerationStructures    = &as;
		    return;
	}
	printVka("Invalid descriptor type!");
	DEBUG_BREAK;
}



void Descriptor::countStructures(uint32_t &bufferInfoCount, uint32_t &imageInfoCount, uint32_t &accelerationStructureWriteCount) const
{
	if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
	{
		bufferInfoCount += count;
		return;
	}
	if (type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	{
		imageInfoCount += count;
		return;
	}
	if (type == VK_DESCRIPTOR_TYPE_SAMPLER || type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	{
		imageInfoCount += count;
		return;
	}
	if (type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
	{
		accelerationStructureWriteCount += 1;
		return;
	}
	printVka("Invalid descriptor type!");
	DEBUG_BREAK;
}
}        // namespace vka