// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\offscreen_swap_chain.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/buffer.h"

#include "Components/graphics_components.h"

#include "vma/vk_mem_alloc.h"
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

/**
 * This is not a real swapchain, since does not need to be presented!
 * But I kept the same logic between offscreen_renderer and offscreen_swap_chain as it is for renderer and swap_chain
 */
class VESPERENGINE_API OffscreenSwapChain final
{
public:
	OffscreenSwapChain(Device& _device, VkExtent2D _imageExtent, VkFormat _imageFormat);
	~OffscreenSwapChain();

	OffscreenSwapChain(const OffscreenSwapChain&) = delete;
	OffscreenSwapChain& operator=(const OffscreenSwapChain&) = delete;

	VESPERENGINE_INLINE const VkFramebuffer GetFrameBuffer() const { return m_framebuffer; }
	VESPERENGINE_INLINE const VkRenderPass GetRenderPass() const { return m_renderPass; }

	VESPERENGINE_INLINE const VkFormat GetSwapChainImageFormat() const { return m_imageFormat; }
	VESPERENGINE_INLINE const VkExtent2D GetSwapChainExtent() const { return m_imageExtent; }
	VESPERENGINE_INLINE const uint32 GetWidth() const { return m_imageExtent.width; }
	VESPERENGINE_INLINE const uint32 GetHeight() const { return m_imageExtent.height; }

	VESPERENGINE_INLINE const VkImage GetOffscreenImage() const { return m_offscreenImage; }

	VESPERENGINE_INLINE float GetExtentAspectRatio()
	{
		return static_cast<float>(m_imageExtent.width) / static_cast<float>(m_imageExtent.height);
	}

	BufferComponent PrepareImageCopy(VkCommandBuffer _commandBuffer);
	void FlushBufferToFile(const std::string& _filePath, BufferComponent& _stagingBuffer);

private:
	void CreateOffscreenImage();
	void CreateRenderPass();
	void CreateFramebuffer();

private:
	VkFormat m_imageFormat;
	VkExtent2D m_imageExtent;
	VkFramebuffer m_framebuffer;
	VkRenderPass m_renderPass;

	Device& m_device;

	VkImage m_offscreenImage;
	VmaAllocation m_offscreenImageMemory;
	VkImageView m_offscreenImageView;

	std::unique_ptr<Buffer> m_buffer;
};

VESPERENGINE_NAMESPACE_END
