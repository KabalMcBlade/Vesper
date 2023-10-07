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

VESPERENGINE_NAMESPACE_END
