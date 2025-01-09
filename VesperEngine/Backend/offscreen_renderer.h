// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\offscreen_renderer.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/offscreen_swap_chain.h"

#include "Components/graphics_components.h"

#include "vma/vk_mem_alloc.h"


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API OffscreenRenderer final
{
public:
	OffscreenRenderer(Device& _device, VkExtent2D _extent, VkFormat _format);
	~OffscreenRenderer();

	OffscreenRenderer(const OffscreenRenderer&) = delete;
	OffscreenRenderer& operator=(const OffscreenRenderer&) = delete;

public:
	VESPERENGINE_INLINE VkRenderPass GetOffscreenSwapChainRenderPass() const { return m_offscreenSwapChain->GetRenderPass(); }
	VESPERENGINE_INLINE float GetAspectRatio() const { return m_offscreenSwapChain->GetExtentAspectRatio(); };
	VESPERENGINE_INLINE bool IsFrameStarted() const { return m_isFrameStarted; }

	VESPERENGINE_INLINE VkCommandBuffer GetCurrentCommandBuffer() const 
	{
		assertMsgReturnValue(IsFrameStarted(), "Cannot get command buffer when frame is not started", VK_NULL_HANDLE);
		return m_commandBuffer;
	}

public:
	VkCommandBuffer BeginFrame();
	void EndFrame();

	void BeginOffscreenSwapChainRenderPass(VkCommandBuffer _commandBuffer);
	void EndOffscreenSwapChainRenderPass(VkCommandBuffer _commandBuffer);

	BufferComponent PrepareImageCopy(VkCommandBuffer _commandBuffer);
	void FlushBufferToFile(const std::string& _filePath, BufferComponent& _stagingBuffer);

private:
	void CreateCommandBuffer();
	void FreeCommandBuffer();

private:
	Device& m_device;
	std::unique_ptr<OffscreenSwapChain> m_offscreenSwapChain;
	VkCommandBuffer m_commandBuffer;

	bool m_isFrameStarted = false;
};

VESPERENGINE_NAMESPACE_END