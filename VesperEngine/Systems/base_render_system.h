// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\base_render_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "vulkan/vulkan.h"

#include <vector>


namespace ecs {
	class ComponentManager;
	class Entity;
}

VESPERENGINE_NAMESPACE_BEGIN

class Device;
struct FrameInfo;
struct IndexBufferComponent;
struct VertexBufferComponent;

class VESPERENGINE_API BaseRenderSystem
{
public:
	BaseRenderSystem(Device& _device);
	virtual ~BaseRenderSystem();

	BaseRenderSystem(const BaseRenderSystem&) = delete;
	BaseRenderSystem& operator=(const BaseRenderSystem&) = delete;

	void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts);

protected:
	virtual void PerEntityUpdate(const FrameInfo& _frameInfo, ecs::ComponentManager& _componentManager, const ecs::Entity& _entity) {}
	virtual void PerEntityRender(const FrameInfo& _frameInfo, ecs::ComponentManager& _componentManager, const ecs::Entity& _entity) {}

    void PushConstants(VkCommandBuffer _commandBuffer, const uint32 _pushConstantIndex, const void* _pushConstantValue) const;
	void PushConstants(VkCommandBuffer _commandBuffer, const uint32 _pushConstantIndex, uint32 _overrideOffset, uint32 _overrideSize, const void* _pushConstantValue) const;
    void PushConstants(VkCommandBuffer _commandBuffer, std::vector<const void*> _pushConstantValues) const;

	void Bind(const VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const;
	void Bind(const VertexBufferComponent& _vertexBufferComponent, const IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const;

	void Draw(const VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer, uint32 _instanceCount = 1) const;
	void Draw(const IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer, uint32 _instanceCount = 1) const;

protected:
	Device& m_device;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    std::vector<VkPushConstantRange> m_pushConstants;
};

VESPERENGINE_NAMESPACE_END
