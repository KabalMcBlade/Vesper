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
// Is also useful to move around or use as buffer containers, as long as is not directly used as component for entity (ECS)
// Could be added to the list, but this is pure for temporary usage
struct BufferComponent
{
	VmaAllocation AllocationMemory{ VK_NULL_HANDLE };	// value storing the allocation to map/unmap	
	VkBuffer Buffer{ VK_NULL_HANDLE };					// value used to bind and draw
	VkDeviceSize Size{ 0 };								// the size (aligned or not) of the current buffer
	uint32 Count{ 0 };									// instance count
};

struct VertexBufferComponent : public BufferComponent
{
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

// define the struct which each instance has at least to have, if needs to be visible/rendered
struct RenderComponent
{
	glm::mat4 ModelMatrix{ 1 };
	uint32 DynamicOffsetIndex { 0 };
	uint32 DynamicOffset { 0 };		// future, we should have a different struct, and having the DynamicOffsetCount and this one as ptr
};

VESPERENGINE_NAMESPACE_END
