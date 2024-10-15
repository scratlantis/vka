#include "Resource.h"
#include <vka/globals.h>
namespace vka
{
void Resource::garbageCollect()
{
	track(gState.frame->stack);
}

void Resource::track(IResourcePool *pPool)
{
	if (!pPool)
	{
		printVka("Null resource pool!");
		DEBUG_BREAK;
		return;
	}
	if (this->pPool)
	{
		if (this->pPool == pPool)
		{
			return;
		}

		if (this->pPool->remove(this))
		{
			this->pPool = pPool;
			this->pPool->add(this);
		}
		else
		{
			printVka("Resource not found in assigned pool!");
			DEBUG_BREAK;
		}
	}
	else
	{
		this->pPool = pPool;
		this->pPool->add(this);
	}
}

void Resource::untrack()
{
	if (this->pPool)
	{
		this->pPool->remove(this);
		this->pPool = nullptr;
	}
}

IResourcePool *Resource::getPool() const
{
	return pPool;
}

}        // namespace vka