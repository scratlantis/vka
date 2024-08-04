#pragma once
#include "../Resource.h"
#include <vma/vk_mem_alloc.h>

namespace vka
{

class Mappable_T : public Resource_T<VkBuffer>
{
  public:
	Mappable_T(VkBuffer handle) :
	    Resource_T<VkBuffer>(handle){};
	virtual void *map(uint32_t offset, uint32_t size) const = 0;
	virtual void  unmap() const                             = 0;
};

class BufferVMA_R : public Mappable_T
{
  public:
	BufferVMA_R(VkBuffer handle, VmaAllocation allocation) :
	    Mappable_T(handle)
	{
		this->allocation = allocation;
	}

	void  free() override;
	void *map(uint32_t offset, uint32_t size) const override;
	void  unmap() const override;

  private:
	VmaAllocation allocation;
};

class BufferView_R : public Resource_T<VkBufferView>
{
  public:
	BufferView_R(VkBufferView handle) :
	    Resource_T<VkBufferView>(handle)
	{}
	void free() override;
};

enum class BufferType
{
	NONE,
	VMA,
	VK
};

union MemoryProperty
{
	VmaMemoryUsage        vma;
	VkMemoryPropertyFlags vk;
};

struct BufferState
{
	BufferType         type;
	VkDeviceSize       size;
	VkBufferUsageFlags usage;
	MemoryProperty     memProperty;
};

struct BufferRange
{
	VkDeviceSize offset = 0;
	VkDeviceSize size = VK_WHOLE_SIZE;
};

class BufferMapping_R : public Resource_T<void *>
{
  public:
	BufferMapping_R(const Mappable_T *mappable, uint32_t offset, uint32_t size);
	void  free() override;
	void *ptr() const;

  private:
	const Mappable_T *m_mappable;
};

class Buffer_R : public Resource_T<VkBuffer>
{
  protected:
	Mappable_T   *res                = nullptr;
	BufferView_R *viewRes            = nullptr;
	bool          isMapped           = false;
	Buffer_R(const Buffer_R &rhs)    = default;
  public:
	VkBufferView viewHandle = VK_NULL_HANDLE;

	// State
	BufferState state;
	BufferState newState;

	// Range
	BufferRange range;

	Buffer_R(IResourcePool *pPool) :
	    Resource_T<VkBuffer>(VK_NULL_HANDLE)
	{
		track(pPool);
		state.type            = BufferType::NONE;
		state.usage           = 0;
		state.memProperty.vma = VMA_MEMORY_USAGE_CPU_ONLY;
		state.size            = 0;
		newState              = state;
	};

	Buffer_R() :
	    Buffer_R(nullptr){};

	Buffer_R(IResourcePool *pPool, VkBufferUsageFlags usage) :
	    Buffer_R(pPool)
	{
		newState.usage = usage;
	};
	~Buffer_R()
	{
		free();
	}

  private:



  public:
	bool   isMappable() const;
	void   track(IResourcePool *pPool) override;
	hash_t hash() const override;

	void *map(uint32_t offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

	void changeSize(VkDeviceSize size);
	void addUsage(VkBufferUsageFlags usage);
	void changeUsage(VkBufferUsageFlags usage);
	void changeMemoryType(VmaMemoryUsage memProperty);

	VkDeviceSize       getSize() const;
	VkBufferUsageFlags getUsage() const;
	VmaMemoryUsage     getMemoryType() const;

	BufferRange     getRange() const;
	const Buffer_R  getSubBuffer(BufferRange range) const;
	const Buffer_R  getShallowCopy() const;
	Buffer_R        getStagingBuffer() const;
	VkDeviceAddress getDeviceAddress() const;
	

	void           free(){};
	void           createHandles();
	void           detachChildResources();
	const Buffer_R recreate();
	void           update();
	friend class BufferMapping;
};
}        // namespace vka
