#include "StructureChain.h"


    namespace vka
{
	

	uint32_t StructureChain::getSize()
	{
		return VKA_COUNT(chain);
	}

	bool StructureChain::empty()
	{
		return chain.empty();
	}
	

	size_t StructureChain::Node::getSize() const
	{
		return size;
	}



	StructureChain::Node::Node(const Node &node) :
	    size(node.size)
	{
		data = copyBinaryData(node.data, node.size);
	}

	StructureChain::Node::~Node()
	{
		delete[] reinterpret_cast<char *>(data);
	}

	VkBaseOutStructure *StructureChain::Node::getNode() const
	{
		return reinterpret_cast<VkBaseOutStructure *>(data);
	}

	

	VkBaseOutStructure *StructureChain::firstNode()
	{
		return chain.empty() ? nullptr : chain.begin()->getNode();
	}

	VkBaseOutStructure *StructureChain::lastNode()
	{
		return chain.empty() ? nullptr : chain.rbegin()->getNode();
	}

	VkBaseOutStructure *StructureChain::chainNodes()
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
	hash_t StructureChain::getHash()
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

}        // namespace vka
