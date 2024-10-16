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




template <class StructureType>
inline void StructureChain::addNode(const StructureType &node)
{
	chain.emplace_back(node);
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
