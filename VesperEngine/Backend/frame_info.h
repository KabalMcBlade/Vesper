// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\frame_info.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

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
	VkDescriptorSet EntityDescriptorSet{ VK_NULL_HANDLE };
};

VESPERENGINE_NAMESPACE_END
