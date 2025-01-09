// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\opaque_render_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/pipeline.h"
#include "Backend/frame_info.h"
#include "Backend/descriptors.h"

#include "Systems/core_render_system.h"

#include "App/vesper_app.h"

#include <memory>

#include "ECS/ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API OpaqueRenderSystem final : public CoreRenderSystem
{
public:
	OpaqueRenderSystem(VesperApp& _app, Device& _device, DescriptorPool& _globalDescriptorPool, VkRenderPass _renderPass,
		VkDescriptorSetLayout _globalDescriptorSetLayout,
		VkDescriptorSetLayout _groupDescriptorSetLayout,
		uint32 _alignedSizeUBO);
	~OpaqueRenderSystem();

	OpaqueRenderSystem(const OpaqueRenderSystem&) = delete;
	OpaqueRenderSystem& operator=(const OpaqueRenderSystem&) = delete;

public:
	VESPERENGINE_INLINE uint32 GetObjectCount() const { return m_internalCounter; }
	VESPERENGINE_INLINE uint32 GetAlignedSizeUBO() const { return m_alignedSizeUBO; }

public:
	void Update(const FrameInfo& _frameInfo);
	void Render(const FrameInfo& _frameInfo);

public:
	void RegisterEntity(ecs::Entity _entity) const;
	void UnregisterEntity(ecs::Entity _entity) const;
	void UnregisterEntities() const;

private:
	void CreatePipeline(VkRenderPass _renderPass);

private:
	VesperApp& m_app;
	DescriptorPool& m_globalDescriptorPool;
	std::unique_ptr<Pipeline> m_opaquePipeline;
	std::unique_ptr<DescriptorSetLayout> m_materialSetLayout;
	uint32 m_alignedSizeUBO {0};
	mutable uint32 m_internalCounter {0};
};

VESPERENGINE_NAMESPACE_END
