// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\core_render_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vulkan/vulkan.h"
#include "Core/core_defines.h"
#include "Backend/device.h"
#include "Components/graphics_components.h"
#include "Components/camera_components.h"
#include "Backend/pipeline.h"
#include "Backend/frame_info.h"

VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API CoreRenderSystem {
public:
	CoreRenderSystem(Device& _device);
	virtual ~CoreRenderSystem();

	CoreRenderSystem(const CoreRenderSystem&) = delete;
	CoreRenderSystem& operator=(const CoreRenderSystem&) = delete;

	void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts);

protected:
	void PushConstants(VkCommandBuffer _commandBuffer, const uint32 _pushConstantIndex, const void* _pushConstantValue) const;
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
