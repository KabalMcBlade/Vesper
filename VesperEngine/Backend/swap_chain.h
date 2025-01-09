// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\swap_chain.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/device.h"

#include "vma/vk_mem_alloc.h"
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API SwapChain final
{
public:
	// SwapChain image count can be 2 or 3, depending if the device support triple buffering
	static constexpr int32 kMaxFramesInFlight = 3;

	SwapChain(Device& _device, VkExtent2D _windowExtent);
	SwapChain(Device& _device, VkExtent2D _windowExtent, std::shared_ptr<SwapChain> _previous);
	~SwapChain();

	SwapChain(const SwapChain&) = delete;
	SwapChain& operator=(const SwapChain&) = delete;

	VESPERENGINE_INLINE const VkFramebuffer GetFrameBuffer(int32 _index) const { return m_swapChainFramebuffers[_index]; }
	VESPERENGINE_INLINE const VkRenderPass GetRenderPass() const { return m_renderPass; }
	VESPERENGINE_INLINE const VkImageView GetImageView(int32 _index) const { return m_swapChainImageViews[_index]; }
	VESPERENGINE_INLINE const std::size_t GetImageCount() const { return m_swapChainImages.size(); }
	VESPERENGINE_INLINE const VkFormat GetSwapChainImageFormat() const { return m_swapChainImageFormat; }
	VESPERENGINE_INLINE const VkExtent2D GetSwapChainExtent() const { return m_swapChainExtent; }
	VESPERENGINE_INLINE const uint32 GetWidth() const { return m_swapChainExtent.width; }
	VESPERENGINE_INLINE const uint32 GetHeight() const { return m_swapChainExtent.height; }

	VESPERENGINE_INLINE float GetExtentAspectRatio() 
	{
		return static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height);
	}

	VESPERENGINE_INLINE bool CompareSwapFormats(const SwapChain& _swapChain) const
	{
		return 
			_swapChain.m_swapChainImageFormat == m_swapChainImageFormat && 
			_swapChain.m_swapChainDepthFormat == m_swapChainDepthFormat;
	}

	VkFormat FindDepthFormat();

	VkResult AcquireNextImage(uint32* _imageIndex);
	VkResult SubmitCommandBuffers(const VkCommandBuffer* _buffers, uint32* _imageIndex);

private:
	void Init();

	void CreateSwapChain();
	void CreateImageViews();
	void CreateDepthResources();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateSyncObjects();

	// Helper functions
	VkSurfaceFormatKHR SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats);
	VkPresentModeKHR SelectSwapPresentMode(const std::vector<VkPresentModeKHR>& _availablePresentModes);
	VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities);

	VkFormat m_swapChainImageFormat;
	VkFormat m_swapChainDepthFormat;
	VkExtent2D m_swapChainExtent;

	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	VkRenderPass m_renderPass;

	std::vector<VkImage> m_depthImages;
	std::vector<VmaAllocation> m_depthImageMemorys;
	std::vector<VkImageView> m_depthImageViews;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;

	Device& m_device;
	VkExtent2D m_windowExtent;

	VkSwapchainKHR m_swapChain;
	std::shared_ptr<SwapChain> m_oldSwapChain;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlight;
	std::size_t m_currentFrame = 0;
};

VESPERENGINE_NAMESPACE_END