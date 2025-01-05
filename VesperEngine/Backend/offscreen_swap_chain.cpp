#include "Backend/offscreen_swap_chain.h"

#include "Components/graphics_components.h"

#include "Utility/logger.h"

#include "../stb/stb_image_write.h"

#include <vector>
#include <cstring>
#include <stdexcept>
#include <array>


VESPERENGINE_NAMESPACE_BEGIN


OffscreenSwapChain::OffscreenSwapChain(Device& _device, VkExtent2D _imageExtent, VkFormat _imageFormat)
	: m_device{ _device }
	, m_imageExtent{ _imageExtent }
	, m_imageFormat{ _imageFormat }
{
	m_buffer = std::make_unique<Buffer>(m_device);

	CreateOffscreenImage();
	CreateRenderPass();
	CreateFramebuffer();
}

OffscreenSwapChain::~OffscreenSwapChain()
{
	vkDestroyFramebuffer(m_device.GetDevice(), m_framebuffer, nullptr);
	vkDestroyRenderPass(m_device.GetDevice(), m_renderPass, nullptr);
	vkDestroyImageView(m_device.GetDevice(), m_offscreenImageView, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_offscreenImage, m_offscreenImageMemory);
}

void OffscreenSwapChain::CreateOffscreenImage() 
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = { m_imageExtent.width, m_imageExtent.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = m_imageFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	m_device.CreateImageWithInfo(imageInfo, m_offscreenImage, m_offscreenImageMemory, VMA_MEMORY_USAGE_GPU_ONLY);

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_offscreenImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = m_imageFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(m_device.GetDevice(), &viewInfo, nullptr, &m_offscreenImageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create off-screen image view!");
	}
}

void OffscreenSwapChain::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// Add subpass dependencies
	std::array<VkSubpassDependency, 2> dependencies{};

	// Dependency from external to subpass
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Dependency from subpass to external
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(m_device.GetDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create off-screen render pass!");
	}
}

void OffscreenSwapChain::CreateFramebuffer()
{
	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = m_renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &m_offscreenImageView;
	framebufferInfo.width = m_imageExtent.width;
	framebufferInfo.height = m_imageExtent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(
		m_device.GetDevice(),
		&framebufferInfo,
		nullptr,
		&m_framebuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create off-screen framebuffer!");
	}
}


BufferComponent OffscreenSwapChain::PrepareImageCopy(VkCommandBuffer _commandBuffer)
{
	VkDeviceSize imageSize = m_imageExtent.width * m_imageExtent.height * 4; // Assuming 4 bytes per pixel (RGBA)

	BufferComponent stagingBuffer = m_buffer->Create<BufferComponent>(
		imageSize,
		1,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY,	//VMA_MEMORY_USAGE_AUTO_PREFER_HOST
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	m_device.CopyImageToBuffer(_commandBuffer, m_offscreenImage, stagingBuffer.Buffer, m_imageExtent.width, m_imageExtent.height, 1);

	return stagingBuffer;
}

void OffscreenSwapChain::FlushBufferToFile(const std::string& _filePath, BufferComponent& _stagingBuffer)
{
	m_buffer->Map(_stagingBuffer);
	m_buffer->WriteToBuffer(_stagingBuffer);
	stbi_write_png(_filePath.c_str(), m_imageExtent.width, m_imageExtent.height, 4, _stagingBuffer.MappedMemory, m_imageExtent.width * 4);
	m_buffer->Unmap(_stagingBuffer);
	m_buffer->Destroy(_stagingBuffer);
}

VESPERENGINE_NAMESPACE_END
