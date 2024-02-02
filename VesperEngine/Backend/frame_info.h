#pragma once

#include "Core/core_defines.h"

#include "vulkan/vulkan.h"

VESPERENGINE_NAMESPACE_BEGIN

struct FrameInfo
{
	int32 FrameIndex{ 0 };
	float FrameTime{ 0.0f };
	VkCommandBuffer CommandBuffer{ VK_NULL_HANDLE };
	VkDescriptorSet GlobalDescriptorSet{ VK_NULL_HANDLE };
	VkDescriptorSet GroupDescriptorSet{ VK_NULL_HANDLE };
};

VESPERENGINE_NAMESPACE_END
