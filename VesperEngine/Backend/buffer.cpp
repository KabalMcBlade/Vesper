#include "Backend/buffer.h"

#include <cassert>
#include <cstring>


VESPERENGINE_NAMESPACE_BEGIN


Buffer::Buffer(Device& device) : m_device{ device }
{
}

void Buffer::WriteToBuffer(void* _outMappedData, void* _inData, VkDeviceSize _size)
{
	assertMsgReturnVoid(_outMappedData, "Cannot copy to unmapped buffer");

	MemCpy(_outMappedData, _inData, _size);
}

void Buffer::WriteToBufferWithOffset(void* _outMappedData, void* _inData, VkDeviceSize _size, VkDeviceSize _offset)
{
	assertMsgReturnVoid(_outMappedData, "Cannot copy to unmapped buffer");

	uint8* memOffset = (uint8*)_outMappedData;
	memOffset += _offset;
	MemCpy(memOffset, _inData, _size);
}

VESPERENGINE_NAMESPACE_END
