#pragma once

#include "vulkan/vulkan.h"

#include "Backend/device.h"

#include "Core/core_defines.h"
#include "Core/memory_copy.h"

#include <vma/vk_mem_alloc.h>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL Buffer final
{
public:
	Buffer(Device& _device);
	~Buffer() = default;

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

public:
	//////////////////////////////////////////////////////////////////////////
	// TEMPLATE FUNCTIONS

	template<typename BufferType>
	void Create(
		BufferType& _outBufferObject,
		VkDeviceSize _instanceSize,					// size of how many instance we want
		uint32 _instanceCount,						// count of the instance we want
		VkBufferUsageFlags _usageFlags,				// vertex, index, transfer, various bits, etc..., such VK_BUFFER_USAGE_VERTEX_BUFFER_BIT or VK_BUFFER_USAGE_TRANSFER_DST_BIT
		VmaMemoryUsage _memoryUsage,				// memory flags for VAM, such VMA_MEMORY_USAGE_AUTO_PREFER_HOST, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, etc...
		VmaAllocationCreateFlags _allocationFlags,	// allocation flag for VAM, such VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
		VkDeviceSize _minOffsetAlignment = 1,		// min alignment is not required for vertex and index buffer
		bool _isPersistent = false)					// if true, creates a dynamic memory, such the Uniform Buffer, to use in case of frequent write on CPU and frequent read on GPU.
	{
		_outBufferObject.Size = _instanceSize;
		if (_minOffsetAlignment > 1)
		{
			_outBufferObject.AlignedSize = GetAlignment(_outBufferObject.Size, _minOffsetAlignment);
			const VkDeviceSize bufferSize = _outBufferObject.AlignedSize * _instanceCount;
			m_device.CreateBufferWithAlignment(
				bufferSize,
				_usageFlags,
				_memoryUsage,
				_allocationFlags,
				_outBufferObject.Buffer,
				_outBufferObject.AllocationMemory,
				_outBufferObject.AlignedSize,
				_isPersistent
			);
		}
		else
		{
			_outBufferObject.AlignedSize = _outBufferObject.Size;
			const VkDeviceSize bufferSize = _outBufferObject.Size * _instanceCount;
			m_device.CreateBuffer(
				bufferSize,
				_usageFlags,
				_memoryUsage,
				_allocationFlags,
				_outBufferObject.Buffer,
				_outBufferObject.AllocationMemory,
				_isPersistent
			);
		}
	}

	template<typename BufferType>
	void Destroy(BufferType& _buffer)
	{
		//Unmap(_buffer);
		vmaDestroyBuffer(m_device.GetAllocator(), _buffer.Buffer, _buffer.AllocationMemory);
	}

	// NOT NEED IT FOR PERSISTENT BUFFER!
	template<typename BufferType>
	VkResult Map(BufferType& _buffer, void** _outMappedData)
	{
		assertMsgReturnValue(_buffer.Buffer && _buffer.AllocationMemory, "Called map on buffer before create", VK_ERROR_UNKNOWN);
		return vmaMapMemory(m_device.GetAllocator(), _buffer.AllocationMemory, _outMappedData);
	}

	// NOT NEED IT FOR PERSISTENT BUFFER!
	template<typename BufferType>
	void Unmap(BufferType& _buffer)
	{
		assertMsgReturnVoid(_buffer.Buffer && _buffer.AllocationMemory, "Called unmap on buffer before create");
		vmaUnmapMemory(m_device.GetAllocator(), _buffer.AllocationMemory);
	}

	// PERSISTEN BUFFER VERSION
	template<typename BufferType>
	void WriteToBuffer(BufferType& _buffer, void* _outData)
	{
		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(m_device.GetAllocator(), _buffer.AllocationMemory, &allocationInfo);

		WriteToBuffer(allocationInfo.pMappedData, _outData, _buffer.AlignedSize);
	}

	// PERSISTEN BUFFER VERSION
	template<typename BufferType>
	void WriteToBufferWithOffset(BufferType& _buffer, void* _outData, VkDeviceSize _offset)
	{
		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(m_device.GetAllocator(), _buffer.AllocationMemory, &allocationInfo);

		WriteToBufferWithOffset(allocationInfo.pMappedData, _outData, _buffer.AlignedSize, _offset);
	}

	template<typename BufferType>
	VkResult Flush(BufferType& _buffer, VkDeviceSize _offset = 0)
	{
		return vmaFlushAllocation(m_device.GetAllocator(), _buffer.AllocationMemory, _offset, _buffer.AlignedSize);
	}

	template<typename BufferType>
	VkResult Invalidate(BufferType& _buffer, VkDeviceSize _offset)
	{
		return vmaInvalidateAllocation(m_device.GetAllocator(), _buffer.AllocationMemory, _offset, _buffer.AlignedSize);
	}

	template<typename BufferType>
	VkDescriptorBufferInfo GetDescriptorInfo(BufferType& _buffer, VkDeviceSize _offset = 0)
	{
		return VkDescriptorBufferInfo
		{
			_buffer.Buffer,
			_offset,
			_buffer.AlignedSize,
		};
	}

	template<typename BufferType>
	void WriteToIndex(BufferType& _buffer, void* _outData, int32 _index)
	{
		WriteToBufferWithOffset(_buffer, _outData, _index * _buffer.AlignedSize);
	}

	template<typename BufferType>
	VkResult FlushIndex(BufferType& _buffer, int32 _index)
	{
		return Flush(_buffer, _index * _buffer.AlignedSize);
	}

	template<typename BufferType>
	VkDescriptorBufferInfo GetDescriptorInfoForIndex(BufferType& _buffer, int32 _index)
	{
		return GetDescriptorInfo(_buffer, _index * _buffer.AlignedSize);
	}

	template<typename BufferType>
	VkResult InvalidateIndex(BufferType& _buffer, int32 _index)
	{
		return Invalidate(_buffer, _index * _buffer.AlignedSize);
	}
	
	template<typename BufferTypeFrom, typename BufferTypeTo>
	void Copy(BufferTypeFrom& _bufferFrom, BufferTypeTo& _bufferTo, VkDeviceSize _size)
	{
		m_device.CopyBuffer(_bufferFrom.Buffer, _bufferTo.Buffer, _size);
	}

	//////////////////////////////////////////////////////////////////////////
	// NON-TEMPLATE FUNCTIONS

	void WriteToBuffer(void* _inMappedData, void* _outData, VkDeviceSize _size);
	void WriteToBufferWithOffset(void* _inMappedData, void* _outData, VkDeviceSize _size, VkDeviceSize _offset);


private:
	static VkDeviceSize GetAlignment(VkDeviceSize _instanceSize, VkDeviceSize _minOffsetAlignment);

	Device& m_device;
};

VESPERENGINE_NAMESPACE_END
