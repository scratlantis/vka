#include "buffer_utility.h"
#include <vka/globals.h>
namespace vka
{

Buffer createStagingBuffer()
{
	Buffer buf = new Buffer_R(gState.frame->stack, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	buf->changeMemoryType(VMA_MEMORY_USAGE_CPU_ONLY);
	return buf;
}

Buffer createBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags)
{
	return new Buffer_R(pPool, usageFlags);
}

Buffer createBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage)
{
	Buffer buf = new Buffer_R(pPool, usageFlags);
	buf->changeMemoryType(memoryUsage);
	return buf;
}

Buffer createBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VkDeviceSize size)
{
	Buffer buf = new Buffer_R(pPool, usageFlags);
	buf->changeMemoryType(memoryUsage);
	buf->changeSize(size);
	buf->recreate();
	return buf;
}

// Write only
void* writePtr(Buffer buf, uint32_t size)
{
	buf->addUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	buf->changeSize(size);
	buf->changeMemoryType(VMA_MEMORY_USAGE_CPU_ONLY);
	buf->recreate();
	return buf->map(0, size);
}

void write(Buffer buf, const void *data, uint32_t size)
{
	memcpy(writePtr(buf, size), data, size);
}

void fill(Buffer buf, const void *data, uint32_t size, uint32_t count)
{
	VKA_ASSERT(buf->getSize() >= size * count);
	void *mapping = writePtr(buf, size * count);
	for (size_t i = 0; i < count; i++)
	{
		memcpy((char*)mapping + i * size, data, size);
	}
}
}        // namespace vka