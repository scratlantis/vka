#include "HdrImagePdfCache.h"

namespace vka
{
	namespace pbr
	{

	Buffer HdrImagePdfCache::fetch(CmdBuffer cmdBuf, const ImagePdfKey &key)
    {
	    return nullptr; // TODO
    }

    void HdrImagePdfCache::clear()
    {
	    for (auto &pdf : map)
	    {
		    pdf.second->garbageCollect();
	    }
	    map.clear();
    }
	}		// namespace pbr
}		// namespace vka