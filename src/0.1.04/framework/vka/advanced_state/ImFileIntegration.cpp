#include "ImFileIntegration.h"
#include <vka/globals.h>
#include <ImFileDialog.h>

namespace vka
{

void initImFile()
{
	ifd::FileDialog::Instance().CreateTexture = [](uint8_t *data, int w, int h, char fmt) -> void * {
		Buffer hostBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, w * h * 4);
		write(hostBuffer, data, w * h * 4);
		VkFormat   format = (fmt == 0) ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
		Image      img    = createImage(gState.heap, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VkExtent2D{static_cast<uint32_t>(w), static_cast<uint32_t>(h)});
		UploadTask upload{};
		upload.type = UPLOAD_TYPE_TEXTURE;
		upload.pSrc = hostBuffer;
		upload.pDst = img;
		upload.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		gState.uploadQueue->addUploadTask(upload);
		return gState.imguiTextureIDCache->fetch(img);
	};
	ifd::FileDialog::Instance().DeleteTexture = [](void *tex){
		gState.imguiTextureIDCache->garbage(reinterpret_cast<ImTextureID>(tex));
	};
}

}