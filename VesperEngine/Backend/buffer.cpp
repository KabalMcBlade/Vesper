// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\buffer.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Backend/buffer.h"

#include "Core/memory_copy.h"

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
