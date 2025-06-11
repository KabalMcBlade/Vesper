// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\offscreen_renderer.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Backend/offscreen_renderer.h"
#include "Backend/device.h"

#include "Utility/logger.h"

#include <array>


VESPERENGINE_NAMESPACE_BEGIN

OffscreenRenderer::OffscreenRenderer(Device& _device, VkExtent2D _extent, VkFormat _format, uint32 _imageLayerCount)
	: m_device { _device }
{
	m_offscreenSwapChain = std::make_unique<OffscreenSwapChain>(_device, _extent, _format, _imageLayerCount);
	CreateCommandBuffer();
}

OffscreenRenderer::~OffscreenRenderer()
{
	FreeCommandBuffer();
}

void OffscreenRenderer::CreateCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	allocInfo.commandPool = m_device.GetCommandPool();
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(m_device.GetDevice(), &allocInfo, &m_commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffer!");
	}
}

void OffscreenRenderer::FreeCommandBuffer()
{
	vkFreeCommandBuffers(m_device.GetDevice(), m_device.GetCommandPool(), 1, &m_commandBuffer);
}

VkCommandBuffer OffscreenRenderer::BeginFrame()
{
	assertMsgReturnValue(!IsFrameStarted(), "Cannot call begin frame while is already in progress", VK_NULL_HANDLE);

	m_isFrameStarted = true;

	auto commandBuffer = GetCurrentCommandBuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	
	return commandBuffer;
}

void OffscreenRenderer::EndFrame()
{
	assertMsgReturnVoid(IsFrameStarted(), "Cannot call end frame while the frame is not in progress");

	auto commandBuffer = GetCurrentCommandBuffer();

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_device.GetGraphicsQueue());

	m_isFrameStarted = false;
}

void OffscreenRenderer::BeginOffscreenSwapChainRenderPass(VkCommandBuffer _commandBuffer, uint32 _baseLayerIndex, uint32 _layerCount, uint32 _mipLevel)
{
	assertMsgReturnVoid(IsFrameStarted(), "Cannot call BeginOffscreenSwapChainRenderPass while the frame is not in progress");
	assertMsgReturnVoid(_commandBuffer == GetCurrentCommandBuffer(), "Cannot begin render pass on command buffer from a different frame");

	// we record the render pass to begin with
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_offscreenSwapChain->GetRenderPass();
	renderPassInfo.framebuffer = m_offscreenSwapChain->GetFrameBuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_offscreenSwapChain->GetSwapChainExtent();

	VkClearValue clearColor{};
	clearColor.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_offscreenSwapChain->GetSwapChainExtent().width);
	viewport.height = static_cast<float>(m_offscreenSwapChain->GetSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_offscreenSwapChain->GetSwapChainExtent();

	vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);
}

void OffscreenRenderer::EndOffscreenSwapChainRenderPass(VkCommandBuffer _commandBuffer)
{
	assertMsgReturnVoid(IsFrameStarted(), "Cannot call EndSwapChainRenderPass while the frame is not in progress");
	assertMsgReturnVoid(_commandBuffer == GetCurrentCommandBuffer(), "Cannot end render pass on command buffer from a different frame");

	vkCmdEndRenderPass(_commandBuffer);
}


BufferComponent OffscreenRenderer::PrepareImageCopy(VkCommandBuffer _commandBuffer)
{
	return m_offscreenSwapChain->PrepareImageCopy(_commandBuffer);
}

void OffscreenRenderer::FlushBufferToFile(const std::string& _filePath, BufferComponent& _stagingBuffer)
{
	m_offscreenSwapChain->FlushBufferToFile(_filePath, _stagingBuffer);
}

VESPERENGINE_NAMESPACE_END
