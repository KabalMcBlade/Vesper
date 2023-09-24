#pragma once

#include "Core/core_defines.h"


#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vma/vk_mem_alloc.h>


VESPERENGINE_NAMESPACE_BEGIN

struct RenderComponent
{
	VkBuffer VertexBuffer {VK_NULL_HANDLE};
	VkDeviceSize VertexOffset {0};
	VmaAllocation VertexBufferMemory{ VK_NULL_HANDLE };
	uint32 VertexCount{ 0 };
};

struct TransformComponent
{
	glm::vec4 Position { 0.0f, 0.0f, 0.0f, 1.0f };
	glm::vec4 Scale { 1.f };
	glm::quat Rotation {};
};

struct MaterialComponent
{
	glm::vec4 Color;
};

VESPERENGINE_NAMESPACE_END
