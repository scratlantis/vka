#pragma once
#include <vka/core/core_common.h>
#include <unordered_map>
namespace vka
{
namespace pbr
{
struct ImagePdfKey
{
	std::string name;
	glm::uvec2  subdivisions;

	bool operator==(const ImagePdfKey &other) const
	{
		return name == other.name && subdivisions == other.subdivisions;
	}

	vka::hash_t hash() const
	{
		return std::hash<std::string>()(name) ^ subdivisions.x ^ subdivisions.y;
	}
};
}        // namespace pbr
}        // namespace vka
DECLARE_HASH(vka::pbr::ImagePdfKey, hash);


namespace vka
{
namespace pbr
{
    class HdrImagePdfCache
    {
		IResourcePool                          *pPool;
		std::unordered_map<ImagePdfKey, Buffer> map;

	  public:
		HdrImagePdfCache(IResourcePool *pPool) :
		    pPool(pPool){};

		Buffer fetch(CmdBuffer cmdBuf, const ImagePdfKey &key);

		void clear();
    };

	}
}