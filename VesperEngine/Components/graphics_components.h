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

// Pure struct, means is not registered and MUST not be registered.
// Used as base structures for buffers
struct BufferComponent
{
	VkBuffer Buffer{ VK_NULL_HANDLE };					// value used to bind and draw
	VmaAllocation AllocationMemory{ VK_NULL_HANDLE };	// value storing the allocation to map/unmap							
};


struct VertexBufferComponent : public BufferComponent
{
	uint32 Count{ 0 };
};

// Special struct to set if we want to mark a render-able object having NOT a VB.
// To be fair, does not make sense, but to be aligned with the IB, I added as well.
// 
// IMPORTANT!!!
// This struct is a "toggle", NEED to be add IF AND ONLY IF VertexBufferComponent IS NOT PRESENT
struct NotVertexBufferComponent
{

};

struct IndexBufferComponent : public BufferComponent
{
	uint32 Count{ 0 };
};

// 
// IMPORTANT!!!
// This struct is a "toggle", NEED to be add IF AND ONLY IF IndexBufferComponent IS NOT PRESENT
struct NotIndexBufferComponent
{

};

struct MaterialComponent
{
	glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };	// opaque white
};

VESPERENGINE_NAMESPACE_END
