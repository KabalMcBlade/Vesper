#include "pch.h"

#include "Systems/core_render_system.h"


VESPERENGINE_NAMESPACE_BEGIN

CoreRenderSystem::CoreRenderSystem(Device& _device)
	: m_device{ _device }
{
}

CoreRenderSystem::~CoreRenderSystem()
{
}

void CoreRenderSystem::Bind(RenderComponent& _renderComponent, VkCommandBuffer _commandBuffer)
{
	VkBuffer buffers[] = { _renderComponent.VertexBuffer };
	VkDeviceSize offsets[] = { _renderComponent.VertexOffset };

	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, buffers, offsets);
}

void CoreRenderSystem::Draw(RenderComponent& _renderComponent, VkCommandBuffer _commandBuffer)
{
	vkCmdDraw(_commandBuffer, _renderComponent.VertexCount, 1, 0, 0);
}

VESPERENGINE_NAMESPACE_END
