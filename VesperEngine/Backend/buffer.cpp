#include "pch.h"
#include "Backend/buffer.h"

#include <cassert>
#include <cstring>


VESPERENGINE_NAMESPACE_BEGIN

VkDeviceSize Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) 
{
    if (minOffsetAlignment > 0) 
    {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }

    return instanceSize;
}

Buffer::Buffer(Device& device) : m_device{ device }
{
}

void Buffer::WriteToBuffer(void* _inMappedData, void* _outData, VkDeviceSize _size)
{
	assertMsgReturnVoid(_inMappedData, "Cannot copy to unmapped buffer");

	MemCpy(_inMappedData, _outData, _size);
}

void Buffer::WriteToBufferWithOffset(void* _inMappedData, void* _outData, VkDeviceSize _size, VkDeviceSize _offset)
{
	assertMsgReturnVoid(_inMappedData, "Cannot copy to unmapped buffer");

	uint8* memOffset = (uint8*)_inMappedData;
	memOffset += _offset;
	MemCpy(memOffset, _outData, _size);
}

VESPERENGINE_NAMESPACE_END
