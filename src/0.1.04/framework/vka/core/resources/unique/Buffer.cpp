#include "Buffer.h"
#include <vka/globals.h>

namespace vka
{

void BufferVMA_R::free()
{
	gState.memAlloc.destroyBuffer(handle, allocation);
}

void *BufferVMA_R::map(uint32_t offset, uint32_t size) const
{
	void *data;
	gState.memAlloc.mapMemory(allocation, &data);
	data = (char *) data + offset;
	return data;
}
void BufferVMA_R::unmap() const
{
	gState.memAlloc.unmapMemory(allocation);
}

void BufferView_R::free()
{
	vkDestroyBufferView(gState.device.logical, handle, nullptr);
}

void Buffer_R::createHandles()
{
	state.type = BufferType::VMA; // Only VMA buffers are supported for now
	VmaAllocation allocation;
	gState.memAlloc.createBuffer(state.size, state.usage, state.memProperty.vma, &handle, &allocation);
	res = new BufferVMA_R(handle, allocation);
	res->track(getPool());
}

void Buffer_R::detachChildResources()
{
	if (res)
	{
		res->track(gState.frame->stack);
		res = nullptr;
	}
	if (viewRes)
	{
		viewRes->track(gState.frame->stack);
		viewRes = nullptr;
	}
}

bool Buffer_R::isMappable() const
{
	//clang-format off
	return !(state.type == BufferType::NONE || (state.type == BufferType::VMA && state.memProperty.vma == VMA_MEMORY_USAGE_GPU_ONLY) || (state.type == BufferType::VK && !(state.memProperty.vk & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)));
	//clang-format on
}

const Buffer_R Buffer_R::recreate()
{
	Buffer_R bufferCopy = *this;
	if (handle != VK_NULL_HANDLE && state.size == newState.size && state.usage == newState.usage && state.memProperty.vma == newState.memProperty.vma)
	{
		return bufferCopy;
	}
	state = newState;
	detachChildResources();
	createHandles();
	return bufferCopy;
}



void Buffer_R::update()
{
	const Buffer_R oldBuffer   = recreate();
	VkDeviceSize   minDataSize = std::min(oldBuffer.getSize(), getSize());
	void* data_old            = oldBuffer.map(0, minDataSize);
	void* data_new			= this->map(0, minDataSize);
	std::memcpy(data_new, data_old, minDataSize);
}


void Buffer_R::track(IResourcePool *pPool)
{
	if (!pPool)
	{
		printVka("Null resource pool!");
		DEBUG_BREAK;
		return;
	}
	if (viewRes)
	{
		viewRes->track(pPool);
	}
	if (res)
	{
		res->track(pPool);
	}
	Resource::track(pPool);
}
hash_t Buffer_R::hash() const
{
	return res->hash() << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}

void* Buffer_R::map(uint32_t offset, VkDeviceSize size) const
{
	return (new BufferMapping_R(res, offset + range.offset, std::min(size, getSize())))->ptr();
}

void Buffer_R::changeSize(VkDeviceSize size)
{
	newState.size = size;
}

void Buffer_R::addUsage(VkBufferUsageFlags usage)
{
	newState.usage |= usage;
}

void Buffer_R::changeUsage(VkBufferUsageFlags usage)
{
	newState.usage = usage;
}

void Buffer_R::changeMemoryType(VmaMemoryUsage memProperty)
{
	newState.memProperty.vma = memProperty;
}

VkDeviceSize Buffer_R::getSize() const
{
	return std::min(range.size, state.size);
}
VkBufferUsageFlags Buffer_R::getUsage() const
{
	return state.usage;
}

VmaMemoryUsage Buffer_R::getMemoryType() const
{
	return state.memProperty.vma;
}

BufferRange Buffer_R::getRange() const
{
	return range;
}

const Buffer_R *Buffer_R::getSubBuffer(BufferRange range) const
{
	Buffer_R *subBuffer = new Buffer_R(*this);
	subBuffer->range	= range;
	VKA_ASSERT(subBuffer->getPool() == nullptr);
	// Ugly hack to avoid double tracking
	// larger structual problem, will fix in next refactor
	subBuffer->res = nullptr;
	subBuffer->viewRes = nullptr;
	subBuffer->track(gState.frame->stack);
	subBuffer->res = res;
	subBuffer->viewRes = viewRes;
	return subBuffer;
}

VkDeviceAddress Buffer_R::getDeviceAddress() const
{
	VkBufferDeviceAddressInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
	bufferInfo.buffer = handle;
	return vkGetBufferDeviceAddress(gState.device.logical, &bufferInfo);
}

BufferMapping_R::BufferMapping_R(const Mappable_T *mappable, uint32_t offset, uint32_t size):m_mappable(mappable)
{
	handle = m_mappable->map(offset, size);
	track(gState.frame->stack);
}

void BufferMapping_R::free()
{
	m_mappable->unmap();
}

void *BufferMapping_R::ptr() const
{
	return handle;
}

}        // namespace vka