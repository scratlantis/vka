#pragma once
#include <vka/core/core_common.h>

namespace vka
{
enum UploadType
{
	UPLOAD_TYPE_TEXTURE,
};
struct UploadTask
{
	UploadType type;
	Resource *pSrc;
	Resource *pDst;
	VkImageLayout targetLayout;
};
class UploadQueue
{
	std::vector<UploadTask> tasks;
	public:
	UploadQueue() = default;
	~UploadQueue() = default;

	void addUploadTask(UploadTask task);
	void processUploadTasks(CmdBuffer cmdBuf);

};
}        // namespace vka