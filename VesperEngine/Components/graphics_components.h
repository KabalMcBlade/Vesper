#pragma once

#include "Core/core_defines.h"


#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vma/vk_mem_alloc.h>


VESPERENGINE_NAMESPACE_BEGIN

struct VertexBufferComponent
{
	VkBuffer Buffer {VK_NULL_HANDLE};
	VmaAllocation BufferMemory{ VK_NULL_HANDLE };
	VkDeviceSize Offset{ 0 };
	uint32 Count{ 0 };
};

struct IndexBufferComponent
{
	VkBuffer Buffer{ VK_NULL_HANDLE };
	VmaAllocation BufferMemory{ VK_NULL_HANDLE };
	VkDeviceSize Offset{ 0 };
	uint32 Count{ 0 };
};

struct MaterialComponent
{
	glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };	// opaque white
};

VESPERENGINE_NAMESPACE_END
