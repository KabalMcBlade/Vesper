// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\entity_handler_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Backend/descriptors.h"

#include "ECS/ECS/entity.h"

#include "vulkan/vulkan.h"

#include <vector>
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;
class Renderer;
class Buffer;

struct FrameInfo;
struct BufferComponent;

class VESPERENGINE_API EntityHandlerSystem
{
public:
	static constexpr uint32 kEntityBindingIndex = 0u;

public:
	EntityHandlerSystem(VesperApp& _app, Device& _device, Renderer& _renderer);
	virtual ~EntityHandlerSystem() = default;

	EntityHandlerSystem(const EntityHandlerSystem&) = delete;
	EntityHandlerSystem& operator=(const EntityHandlerSystem&) = delete;

public:
	VESPERENGINE_INLINE VkDescriptorSetLayout GetEntityDescriptorSetLayout() const { return m_entitySetLayout->GetDescriptorSetLayout(); }
	VESPERENGINE_INLINE VkDescriptorSet GetEntityDescriptorSet(const int32 _frameIndex) const { return m_entityDescriptorSets[_frameIndex]; }

	VESPERENGINE_INLINE uint32 GetEntityCount() const { return m_internalCounter; }
	VESPERENGINE_INLINE uint32 GetAlignedSizeUBO() const { return m_alignedSizeUBO; }

public:
	// Call this at the beginning, but after all the constructors of all the system is done
	void Initialize();
	// Register an entity to be valid renderable
	void RegisterRenderableEntity(ecs::Entity _entity) const;
	// Call this within the update the entities.
	void UpdateEntities(const FrameInfo& _frameInfo);
	// Call at the end or at destruction time, anyway after the game loop is done.
	void Cleanup();

private:
	void UnregisterEntities() const;

private:
	VesperApp& m_app;
	Device& m_device;
	Renderer& m_renderer;

	std::unique_ptr<DescriptorSetLayout> m_entitySetLayout;
	std::unique_ptr<Buffer> m_buffer;

	std::vector<BufferComponent> m_entityUboBuffers;
	std::vector<VkDescriptorSet> m_entityDescriptorSets;

	uint32 m_alignedSizeUBO{ 0 };
	mutable uint32 m_internalCounter{ 0 };
};

VESPERENGINE_NAMESPACE_END
