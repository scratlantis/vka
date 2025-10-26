#ifndef MISC_H
#define MISC_H

struct AttributeDescriptor
{
	uint stride;
	uint offset;
	uint alignedSize;
};

uint attributeID(uint id, AttributeDescriptor desc)
{
	return (id * desc.stride + desc.offset)/desc.alignedSize;
}


#endif