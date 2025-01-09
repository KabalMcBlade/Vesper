// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\master_render_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"

#include "Components/graphics_components.h"
#include "Components/camera_components.h"

#include "Backend/pipeline.h"

#include "Backend/frame_info.h"

#include "Systems/core_render_system.h"


VESPERENGINE_NAMESPACE_BEGIN


/**
 * Vulkan Canonical Viewing Volume
 * x is in range of -1 to 1
 * y is in range of -1 to 1
 * z is in range of 0 to 1
 * This is right hand coordinate system
 * Not that positive x is pointing right, positive y is down and positive z is point in to the screen (from screen away)
 * 
 */

class VESPERENGINE_API MasterRenderSystem : public CoreRenderSystem
{
public:
	// the _globalDescriptorSetLayout and _groupDescriptorSetLayout, should be managed by this class! REFACTORING!!
	MasterRenderSystem(Device& _device, VkDescriptorSetLayout _globalDescriptorSetLayout, VkDescriptorSetLayout _groupDescriptorSetLayout);
	virtual ~MasterRenderSystem() = default;

	MasterRenderSystem(const MasterRenderSystem&) = delete;
	MasterRenderSystem& operator=(const MasterRenderSystem&) = delete;
	
public:
	void BindGlobalDescriptor(const FrameInfo& _frameInfo);
	void Cleanup();

public:
	virtual void Update(const FrameInfo& _frameInfo) {}
};

VESPERENGINE_NAMESPACE_END
