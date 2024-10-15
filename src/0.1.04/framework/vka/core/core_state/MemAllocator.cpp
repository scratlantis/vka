#define VMA_IMPLEMENTATION
#include "MemAllocator.h"
#include <vka/globals.h>

namespace vka
{

MemAllocator::MemAllocator()
{
}

void MemAllocator::init()
{
	VKA_CHECK(gState.initBits & STATE_INIT_DEVICE_BIT);
	VKA_CHECK(!(gState.initBits & STATE_INIT_MEMALLOC_BIT));
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice         = gState.device.physical;
	allocatorInfo.device                 = gState.device.logical;
	allocatorInfo.instance               = gState.device.instance;
	allocatorInfo.flags                  = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
	gState.initBits |= STATE_INIT_MEMALLOC_BIT;
}

void MemAllocator::destroy()
{
	VKA_CHECK(gState.initBits & STATE_INIT_MEMALLOC_BIT);
	vmaDestroyAllocator(vmaAllocator);
	gState.initBits &= ~STATE_INIT_MEMALLOC_BIT;
}

MemAllocator::~MemAllocator()
{
}

void MemAllocator::createImage(
    const VkImageCreateInfo *pImageCreateInfo,
    VmaAllocationCreateInfo *pVmaAllocationCreateInfo,
    VkImage                 *pImage,
    VmaAllocation           *pAllocation)
{
	VK_CHECK(vmaCreateImage(this->vmaAllocator, pImageCreateInfo, pVmaAllocationCreateInfo, pImage, pAllocation, nullptr));
}

void MemAllocator::destroyImage(VkImage &image, VmaAllocation &allocation)
{
	vmaDestroyImage(this->vmaAllocator, image, allocation);
}

void MemAllocator::mapMemory(const VmaAllocation &allocation, void **ppData)
{
	vmaMapMemory(this->vmaAllocator, allocation, ppData);
}

void MemAllocator::mapMemory(const VkDeviceMemory &mem, uint32_t offset, uint32_t size, void **ppData)
{
	vkMapMemory(gState.device.logical, mem, offset, size, 0, ppData);
}

void MemAllocator::unmapMemory(const VkDeviceMemory &mem)
{
	vkUnmapMemory(gState.device.logical, mem);
}
void MemAllocator::unmapMemory(const VmaAllocation &allocation)
{
	vmaUnmapMemory(this->vmaAllocator, allocation);
}

void MemAllocator::createBuffer(
    VkBufferCreateInfo      *pBufferCreateInfo,
    VmaAllocationCreateInfo *pVmaAllocationCreateInfo,
    VkBuffer                *pBuffer,
    VmaAllocation           *pAllocation,
    VmaAllocationInfo       *pAllocationInfo)
{
	vmaCreateBuffer(this->vmaAllocator, pBufferCreateInfo, pVmaAllocationCreateInfo, pBuffer, pAllocation, pAllocationInfo);
	VKA_CHECK((pAllocation != nullptr));
}

void MemAllocator::createBuffer(
    VkDeviceSize       deviceSize,
    VkBufferUsageFlags vkBufferUsageFlags,
    VmaMemoryUsage     vmaMemoryUsageFlags,
    VkBuffer          *buf,
    VmaAllocation     *alloc)
{
	VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	bufferInfo.size               = deviceSize + 4; // seems to be a bug in vma, it doesn't allocate the correct size
	bufferInfo.usage              = vkBufferUsageFlags;

	VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
	vmaAllocationCreateInfo.usage                   = vmaMemoryUsageFlags;
	VmaAllocationInfo *pAllocationInfo              = {};
	createBuffer(&bufferInfo, &vmaAllocationCreateInfo, buf, alloc, pAllocationInfo);
}

void MemAllocator::createUnallocatedBuffer(
    VkDeviceSize       deviceSize,
    VkBufferUsageFlags bufferUsageFlags,
    VkBuffer          *buf)
{
	VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	bufferCreateInfo.size                  = deviceSize;
	bufferCreateInfo.usage                 = bufferUsageFlags;
	bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices   = nullptr;
	VK_CHECK(vkCreateBuffer(gState.device.logical, &bufferCreateInfo, nullptr, buf));
}

void MemAllocator::allocateBuffer(
    VkBufferUsageFlags    bufferUsageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkMemoryRequirements  memoryRequirements,
    VkDeviceMemory       *mem)
{
	VkMemoryAllocateInfo memoryAllocateInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	memoryAllocateInfo.allocationSize  = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(gState.device.physical, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	if (bufferUsageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	}
	VK_CHECK(vkAllocateMemory(gState.device.logical, &memoryAllocateInfo, nullptr, mem));
}

void MemAllocator::createBufferDedicated(
    VkDeviceSize          deviceSize,
    VkBufferUsageFlags    bufferUsageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkBuffer             *buf,
    VkDeviceMemory       *mem)
{
	// Create Buffer
	createUnallocatedBuffer(deviceSize, bufferUsageFlags, buf);
	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(gState.device.logical, *buf, &memoryRequirements);
	// Allocate Buffer
	allocateBuffer(bufferUsageFlags, memoryPropertyFlags, memoryRequirements, mem);
	// Bind Buffer
	vkBindBufferMemory(gState.device.logical, *buf, *mem, 0);
}

void MemAllocator::destroyBuffer(VkBuffer &buffer, VmaAllocation &allocation)
{
	vmaDestroyBuffer(this->vmaAllocator, buffer, allocation);
}

void MemAllocator::destroyBuffer(VkBuffer &buffer, VkDeviceMemory &deviceMemory)
{
	vkDestroyBuffer(gState.device.logical, buffer, nullptr);
	vkFreeMemory(gState.device.logical, deviceMemory, nullptr);
}

void MemAllocator::destroyImage(VkImage &image, VkDeviceMemory &deviceMemory)
{
	vkDestroyImage(gState.device.logical, image, nullptr);
	vkFreeMemory(gState.device.logical, deviceMemory, nullptr);
}
}        // namespace vka