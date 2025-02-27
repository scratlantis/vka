#include "UploadQueue.h"

void vka::UploadQueue::addUploadTask(UploadTask task)
{
	tasks.push_back(task);
}

void vka::UploadQueue::processUploadTasks(CmdBuffer cmdBuf)
{
	for (auto &task : tasks)
	{
		if (task.type == UPLOAD_TYPE_TEXTURE)
		{
			VKA_ASSERT(task.pSrc->type() == RESOURCE_TYPE_BUFFER && task.pDst->type() == RESOURCE_TYPE_IMAGE);
			Buffer src = static_cast<Buffer>(task.pSrc);
			Image  dst = static_cast<Image>(task.pDst);

			cmdTransitionLayout(cmdBuf, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 1);
			cmdCopyBufferToImage(cmdBuf, src, dst, 0, 0);
			cmdTransitionLayout(cmdBuf, dst, task.targetLayout, 0, 1);
			src->garbageCollect();
		}
	}
	tasks.clear();
}
