#include "Exporter.h"
#include <vka/globals.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include "CSVWriter.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

//https://github.com/syoyo/tinyexr/blob/release/examples/rgbe2exr/rgbe2exr.cc
void tinyexr_write_exr(const char* path, float* rgba, uint32_t width, uint32_t height)
{
	EXRHeader header;
	InitEXRHeader(&header);
	EXRImage image;
	InitEXRImage(&image);

	image.num_channels = 3;

	std::vector<float> images[3];
	images[0].resize(width * height);
	images[1].resize(width * height);
	images[2].resize(width * height);

	for (int i = 0; i < width * height; i++)
	{
		images[0][i] = rgba[4 * i + 0];
		images[1][i] = rgba[4 * i + 1];
		images[2][i] = rgba[4 * i + 2];
	}

	float *image_ptr[3];
	image_ptr[0] = &(images[2].at(0));        // B
	image_ptr[1] = &(images[1].at(0));        // G
	image_ptr[2] = &(images[0].at(0));        // R

	image.images = (unsigned char **) image_ptr;
	image.width  = width;
	image.height = height;

	header.num_channels = 3;
	header.channels     = (EXRChannelInfo *) malloc(sizeof(EXRChannelInfo) * header.num_channels);
	// Must be BGR(A) order, since most of EXR viewers expect this channel order.
	strncpy(header.channels[0].name, "B", 255);
	header.channels[0].name[strlen("B")] = '\0';
	strncpy(header.channels[1].name, "G", 255);
	header.channels[1].name[strlen("G")] = '\0';
	strncpy(header.channels[2].name, "R", 255);
	header.channels[2].name[strlen("R")] = '\0';

	header.pixel_types           = (int *) malloc(sizeof(int) * header.num_channels);
	header.requested_pixel_types = (int *) malloc(sizeof(int) * header.num_channels);
	for (int i = 0; i < header.num_channels; i++)
	{
		header.pixel_types[i]           = TINYEXR_PIXELTYPE_FLOAT;        // pixel type of input image
		header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF;         // pixel type of output image to be stored in .EXR
	}

	const char *err;
	int         ret = SaveEXRImageToFile(&image, &header, path, &err);
	if (ret != TINYEXR_SUCCESS)
	{
		DEBUG_BREAK
		fprintf(stderr, "Save EXR err: %s\n", err);
	}

	free(header.channels);
	free(header.pixel_types);
	free(header.requested_pixel_types);
}


namespace vka
{
Exporter::Exporter()
{
	exportTasks.resize(gState.io.imageCount);
}

void Exporter::cmdExport(CmdBuffer cmdBuf, ExportTask task)
{
	if (task.pResource->type() == RESOURCE_TYPE_BUFFER
		&& task.bufferInfo.format == EXPORT_BUFFER_FORMAT_FLOAT
		&& task.targetFormat == EXPORT_FORMAT_CSV)
	{
		Buffer             deviceBuffer = static_cast<Buffer>(task.pResource);
		ExportTaskInternal expTask{};
		expTask.path       = task.path;
		expTask.targetFormat = task.targetFormat;
		expTask.bufferInfo = task.bufferInfo;
		expTask.imageInfo.width = 0;
		expTask.hostBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY, deviceBuffer->getSize());
		cmdCopyBuffer(cmdBuf, deviceBuffer, expTask.hostBuffer);
		exportTasks[gState.frame->frameIndex].push_back(expTask);
		return;
	}
	else if (task.pResource->type() == RESOURCE_TYPE_IMAGE)
	{
		Image image = static_cast<Image>(task.pResource);
		VulkanFormatInfo formatInfo = cVkFormatTable.at(image->getFormat());
		ExportTaskInternal expTask{};
		expTask.path             = task.path;
		expTask.targetFormat     = task.targetFormat;
		expTask.imageInfo.width  = image->getWidth();
		expTask.imageInfo.height = image->getHeight();
		expTask.imageInfo.channels = formatInfo.channel_count;
		expTask.imageInfo.bytesPerPixel = formatInfo.size;
		expTask.hostBuffer              = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY, image->getWidth() * image->getHeight() * formatInfo.size);
		cmdCopyImageToBuffer(cmdBuf, image, expTask.hostBuffer);
		exportTasks[gState.frame->frameIndex].push_back(expTask);
		return;

	}
	printf("Unsupported export task\n");
}
void Exporter::processExports()
{
	for (auto &task : exportTasks[gState.frame->frameIndex])
	{
		if (task.targetFormat == EXPORT_FORMAT_CSV && task.bufferInfo.format == EXPORT_BUFFER_FORMAT_FLOAT)
		{
			float        *pData  = static_cast<float *>(task.hostBuffer->map());
			std::ofstream file(task.path);
			CSVWriter     csv;
			for (uint32_t i = 0; i < task.hostBuffer->getSize() / sizeof(float); i += task.bufferInfo.rowLength)
			{
				for (uint32_t j = 0; j < task.bufferInfo.rowLength; j++)
				{
					csv << pData[i + j];
				}
				csv.newRow();
			}
			file << csv;
			file.close();
		}
		else if (task.targetFormat == EXPORT_FORMAT_PNG)
		{
			uint8_t *pData = static_cast<uint8_t *>(task.hostBuffer->map());
			stbi_write_png(task.path.c_str(), task.imageInfo.width, task.imageInfo.height, task.imageInfo.channels, pData, task.imageInfo.width * task.imageInfo.channels);
		}
		else if (task.targetFormat == EXPORT_FORMAT_HDR)
		{
			float *pData = static_cast<float *>(task.hostBuffer->map());
			stbi_write_hdr(task.path.c_str(), task.imageInfo.width, task.imageInfo.height, task.imageInfo.channels, pData);
		}
		else if (task.targetFormat == EXPORT_FORMAT_EXR)
		{
			float *pData = static_cast<float *>(task.hostBuffer->map());
			tinyexr_write_exr(task.path.c_str(), pData, task.imageInfo.width, task.imageInfo.height);
		}
		task.hostBuffer->garbageCollect();
	}
	exportTasks[gState.frame->frameIndex].clear();
}
}        // namespace vka