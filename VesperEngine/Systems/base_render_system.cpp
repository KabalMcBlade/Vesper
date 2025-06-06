// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\base_render_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/base_render_system.h"
#include "Systems/render_subsystem.h"

#include "Backend/device.h"

#include "Components/graphics_components.h"


VESPERENGINE_NAMESPACE_BEGIN

BaseRenderSystem::BaseRenderSystem(Device& _device)
    : m_device{ _device }
{
}

void BaseRenderSystem::AddRenderSubsystem(RenderSubsystem* _subsystem)
{
    if (_subsystem)
    {
        m_renderSubsystems.push_back(_subsystem);
        VkPushConstantRange range = _subsystem->GetPushConstantRange();
        if (range.size > 0)
        {
            m_pushConstants.push_back(range);
        }
    }
}

BaseRenderSystem::~BaseRenderSystem()
{
    if (m_pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(m_device.GetDevice(), m_pipelineLayout, nullptr);
    }
    m_renderSubsystems.clear();
}

void BaseRenderSystem::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32>(_descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = _descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32>(m_pushConstants.size());
	pipelineLayoutInfo.pPushConstantRanges = m_pushConstants.data();

	if (vkCreatePipelineLayout(m_device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}

void BaseRenderSystem::PushConstants(VkCommandBuffer _commandBuffer, const uint32 _pushConstantIndex, const void* _pushConstantValue) const
{
	assertMsgReturnVoid(_pushConstantIndex >= 0 && _pushConstantIndex < m_pushConstants.size(), "_pushConstantIndex out of range!");

	vkCmdPushConstants(_commandBuffer, m_pipelineLayout,
		m_pushConstants[_pushConstantIndex].stageFlags,
		m_pushConstants[_pushConstantIndex].offset,
		m_pushConstants[_pushConstantIndex].size,
		_pushConstantValue);
}

void BaseRenderSystem::PushConstants(VkCommandBuffer _commandBuffer, std::vector<const void*> _pushConstantValues) const
{
	assertMsgReturnVoid(_pushConstantValues.size() == m_pushConstants.size(), "_pushConstantValues size mismatch!");

	const int32 size = static_cast<int32>(_pushConstantValues.size());
	for (int32 i = 0; i < size; ++i)
	{
		PushConstants(_commandBuffer, i, _pushConstantValues[i]);
	}
}

void BaseRenderSystem::Bind(const VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	VkBuffer buffers[] = { _vertexBufferComponent.Buffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, buffers, offsets);
}

void BaseRenderSystem::Bind(const VertexBufferComponent& _vertexBufferComponent, const IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	Bind(_vertexBufferComponent, _commandBuffer);
	vkCmdBindIndexBuffer(_commandBuffer, _indexBufferComponent.Buffer, 0, VK_INDEX_TYPE_UINT32);
}

void BaseRenderSystem::Draw(const VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer, uint32 _instanceCount) const
{
	vkCmdDraw(_commandBuffer, _vertexBufferComponent.Count, _instanceCount, 0, 0);
}

void BaseRenderSystem::Draw(const IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer, uint32 _instanceCount) const
{
	vkCmdDrawIndexed(_commandBuffer, _indexBufferComponent.Count, _instanceCount, 0, 0, 0);
}

VESPERENGINE_NAMESPACE_END
