// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\master_render_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/descriptors.h"
#include "Backend/pipeline.h"
#include "Backend/frame_info.h"

#include "Components/graphics_components.h"
#include "Components/camera_components.h"

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
	static constexpr uint32 kGlobalBindingSceneIndex = 0u;
	static constexpr uint32 kGlobalBindingLightsIndex = 1u;

public:
	MasterRenderSystem(Device& _device);
	virtual ~MasterRenderSystem() = default;

	MasterRenderSystem(const MasterRenderSystem&) = delete;
	MasterRenderSystem& operator=(const MasterRenderSystem&) = delete;

public:
	VESPERENGINE_INLINE VkDescriptorSetLayout GetGlobalDescriptorSetLayout() const { return m_globalSetLayout->GetDescriptorSetLayout(); }
	VESPERENGINE_INLINE VkDescriptorSet GetGlobalDescriptorSet(const int32 _frameIndex) const { return m_globalDescriptorSets[_frameIndex]; }

public:
	// Call this at the beginning, but after all the constructors of all the system is done
	void Initialize(DescriptorPool& _globalDescriptorPool);
	// Call this within the update/render loop. Does not need to be between the swapchain, and need to be done before the render and the BindGlobalDescriptor
	void UpdateScene(const FrameInfo& _frameInfo, const CameraComponent& _cameraComponent);
	// Call this after the UpdateScene, but before every other Render from every other system, this is the global descriptor binding point
	void BindGlobalDescriptor(const FrameInfo& _frameInfo);
	// Call at the end or at destruction time, anyway after the game loop is done.
	void Cleanup();

private:
	std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;

	std::vector<BufferComponent> m_globalSceneUboBuffers;
	std::vector<BufferComponent> m_globalLightsUboBuffers;

	std::vector<VkDescriptorSet> m_globalDescriptorSets;

	std::unique_ptr<Buffer> m_buffer;
};

VESPERENGINE_NAMESPACE_END