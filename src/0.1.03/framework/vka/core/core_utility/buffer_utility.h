#pragma once
#include "types.h"
#include <vka/core/resources/IResourcePool.h>

namespace vka
{
Buffer createStagingBuffer();
Buffer createBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags);
Buffer createBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);
Buffer createBuffer(IResourcePool *pPool, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VkDeviceSize size);

void *writePtr(Buffer buf, uint32_t size);
void  write(Buffer buf, const void *data, uint32_t size);
void  fill(Buffer buf, const void *data, uint32_t size, uint32_t count);

}        // namespace vka