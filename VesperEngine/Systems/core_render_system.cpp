#include "pch.h"

#include "Systems/core_render_system.h"


VESPERENGINE_NAMESPACE_BEGIN

CoreRenderSystem::CoreRenderSystem(Device& _device)
	: m_device{ _device }
{
}

void CoreRenderSystem::Bind(VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	VkBuffer buffers[] = { _vertexBufferComponent.Buffer };
	VkDeviceSize offsets[] = { _vertexBufferComponent.Offset };

	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, buffers, offsets);
}

void CoreRenderSystem::Bind(VertexBufferComponent& _vertexBufferComponent, IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	Bind(_vertexBufferComponent, _commandBuffer);
	vkCmdBindIndexBuffer(_commandBuffer, _indexBufferComponent.Buffer, _indexBufferComponent.Offset, VK_INDEX_TYPE_UINT32);
}

void CoreRenderSystem::Draw(VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	vkCmdDraw(_commandBuffer, _vertexBufferComponent.Count, 1, 0, 0);
}

void CoreRenderSystem::Draw(IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	vkCmdDrawIndexed(_commandBuffer, _indexBufferComponent.Count, 1, 0, 0, 0);
}

VESPERENGINE_NAMESPACE_END
