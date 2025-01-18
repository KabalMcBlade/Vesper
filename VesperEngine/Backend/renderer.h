// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\renderer.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Backend/swap_chain.h"

#include "vma/vk_mem_alloc.h"

#include <memory>
#include <vector>
#include <unordered_map>


VESPERENGINE_NAMESPACE_BEGIN

class SwapChain;
class DescriptorPool;
class Device;
class WindowHandle;

class VESPERENGINE_API Renderer final
{
public:
	Renderer(WindowHandle& _window, Device& _device);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

public:
	VESPERENGINE_INLINE VkRenderPass GetSwapChainRenderPass() const { return m_swapChain->GetRenderPass(); }
	VESPERENGINE_INLINE float GetAspectRatio() const { return m_swapChain->GetExtentAspectRatio(); };
	VESPERENGINE_INLINE DescriptorPool* GetDescriptorPool() const { return m_globalPool.get(); }

	VESPERENGINE_INLINE bool IsFrameStarted() const { return m_isFrameStarted; }
	VESPERENGINE_INLINE VkCommandBuffer GetCurrentCommandBuffer() const 
	{
		assertMsgReturnValue(IsFrameStarted(), "Cannot get command buffer when frame is not started", VK_NULL_HANDLE);
		return m_commandBuffers[GetFrameIndex()];
	}

	VESPERENGINE_INLINE int32 GetFrameIndex() const
	{
		assertMsgReturnValue(IsFrameStarted(), "Cannot get frame index when frame is not started", -1);
		return m_currentFrameIndex;
	}

public:
	void SetupGlobalDescriptors(const std::unordered_map<VkDescriptorType, uint32>& _descriptorTypesAndSize, uint32 _maxSetCount);

	VkCommandBuffer BeginFrame();
	void EndFrame();

	void BeginSwapChainRenderPass(VkCommandBuffer _commandBuffer);
	void BeginSwapChainRenderPass(VkCommandBuffer _commandBuffer, VkViewport _viewport, VkRect2D _scissor);
	void EndSwapChainRenderPass(VkCommandBuffer _commandBuffer);

private:
	void RecreateSwapChain();
	void CreateCommandBuffers();
	void FreeCommandBuffers();

private:
	WindowHandle& m_window;
	Device& m_device;
	std::unique_ptr<SwapChain> m_swapChain;
	std::unique_ptr<DescriptorPool> m_globalPool;
	std::vector<VkCommandBuffer> m_commandBuffers;

	uint32 m_currentImageIndex = 0;
	int32 m_currentFrameIndex = 0;
	bool m_isFrameStarted = false;
};

VESPERENGINE_NAMESPACE_END