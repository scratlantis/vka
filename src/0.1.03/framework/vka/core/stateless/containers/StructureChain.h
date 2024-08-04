#pragma once
#include <list>
#include "../utility/macros.h"
#include "../utility/hash.h"
#include "../utility/misc.h"

// from https://github.com/vcoda/magma
namespace vka
{

class StructureChain
{
  public:
	template <class StructureType>
	void addNode(const StructureType &node);

	VkBaseOutStructure *firstNode();

	VkBaseOutStructure *lastNode();

	uint32_t getSize();

	bool empty();

	VkBaseOutStructure *chainNodes();
	hash_t              getHash();

  private:
	class Node;
	std::list<Node> chain;
};

inline uint32_t StructureChain::getSize()
{
	return VKA_COUNT(chain);
}

inline bool StructureChain::empty()
{
	return chain.empty();
}
class StructureChain::Node
{
  public:
	template <class StructureType>
	Node(const StructureType &node);
	Node(const Node &node);
	~Node();
	VkBaseOutStructure *getNode() const;
	size_t              getSize() const;

  private:
	size_t size;
	void  *data;
};

inline size_t StructureChain::Node::getSize() const
{
	return size;
}

template <class StructureType>
inline StructureChain::Node::Node(const StructureType &node) :
    size(sizeof(StructureType))
{
	static_assert(sizeof(StructureType) > sizeof(VkBaseInStructure),
	              "chain structure size is too little");
	static_assert(std::is_trivially_copyable<StructureType>::value,
	              "chain structure required to be trivially copyable");
	data = copyBinaryData(node);
}

inline StructureChain::Node::Node(const Node &node) :
    size(node.size)
{
	data = copyBinaryData(node.data, node.size);
}

inline StructureChain::Node::~Node()
{
	delete[] reinterpret_cast<char *>(data);
}

inline VkBaseOutStructure *StructureChain::Node::getNode() const
{
	return reinterpret_cast<VkBaseOutStructure *>(data);
}

template <class StructureType>
inline void StructureChain::addNode(const StructureType &node)
{
	chain.emplace_back(node);
}

inline VkBaseOutStructure *StructureChain::firstNode()
{
	return chain.empty() ? nullptr : chain.begin()->getNode();
}

inline VkBaseOutStructure *StructureChain::lastNode()
{
	return chain.empty() ? nullptr : chain.rbegin()->getNode();
}

inline VkBaseOutStructure *StructureChain::chainNodes()
{
	if (chain.empty())
		return nullptr;
	auto head = chain.begin(), curr = head, next = head;
	while (++next != chain.end())
	{
		VkBaseOutStructure *node = curr->getNode();
		node->pNext              = next->getNode();
		curr                     = next;
	}
	VkBaseOutStructure *last = curr->getNode();
	last->pNext              = nullptr;
	return head->getNode();
}

// unsafe
inline hash_t StructureChain::getHash()
{
	if (chain.empty())
		return 0;
	auto   node = chain.cbegin();
	hash_t hash = byteHashPtr(node->getNode());
	while (++node != chain.cend())
	{
		hash_t nodeHash = byteHashPtr(node->getNode());
		hashCombineLocal(hash, nodeHash);
	}
	return hash;
}

#define VKA_SPECIALIZE_STRUCTURE_CHAIN_NODE(StructureType, structureType)         \
	template <>                                                                   \
	inline StructureType *vka::StructureChain::findNode<StructureType>() const    \
	{                                                                             \
		auto it = std::find_if(chain.begin(), chain.end(),                        \
		                       [](auto &it) {                                     \
			                       return (it.getNode()->sType == structureType); \
		                       });                                                \
		if (it != chain.end())                                                    \
			return reinterpret_cast<StructureType *>(it->getNode());              \
		return nullptr;                                                           \
	}

}        // namespace vka
