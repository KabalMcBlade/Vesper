#include "pch.h"

#include "renderer.h"


VESPERENGINE_NAMESPACE_BEGIN

Renderer::Renderer(WindowHandle& _window, Device& _device) 
	: m_window {_window}
	, m_device { _device }
{
	RecreateSwapChain();	// it does create the pipeline as well
	CreateCommandBuffers();
}

Renderer::~Renderer()
{
	FreeCommandBuffers();
}

void Renderer::RecreateSwapChain()
{
	auto extent = m_window.GetExtent();
	while (extent.width == 0 || extent.height == 0)
	{
		extent = m_window.GetExtent();
		m_window.WaitEvents();
	}

	vkDeviceWaitIdle(m_device.GetDevice());

	if (m_swapChain == nullptr)
	{
		m_swapChain = std::make_unique<SwapChain>(m_device, extent);
	}
	else
	{
		std::shared_ptr<SwapChain> oldSwapChain = std::move(m_swapChain);
		m_swapChain = std::make_unique<SwapChain>(m_device, extent, oldSwapChain);

		if (!oldSwapChain->CompareSwapFormats(*m_swapChain.get()))
		{
			throw std::runtime_error("Swap chain image or depth format has change!");
		}
	}
}

void Renderer::CreateCommandBuffers()
{
	// SwapChain image count can be 2 or 3, depending if the device support triple buffering
	// Each command buffer is going to draw to a different frame buffer
	m_commandBuffers.resize(SwapChain::kMaxFramesInFlight);

#ifdef _DEBUG
	if (m_commandBuffers.size() <= 2)
	{
		std::cout << "Device support double buffering." << std::endl;
	}
	else
	{
		std::cout << "Device support triple buffering." << std::endl;
	}
#endif

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

	// VK_COMMAND_BUFFER_LEVEL_PRIMARY: can be submitted to a queue for execution, but cannot be called by other command buffers
	// VK_COMMAND_BUFFER_LEVEL_SECONDARY: cannot be submitted, but can be called by other command buffers
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	allocInfo.commandPool = m_device.GetCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32>(m_commandBuffers.size());

	if (vkAllocateCommandBuffers(m_device.GetDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void Renderer::FreeCommandBuffers()
{
	vkFreeCommandBuffers(m_device.GetDevice(), m_device.GetCommandPool(), static_cast<uint32>(m_commandBuffers.size()), m_commandBuffers.data());
	m_commandBuffers.clear();
}

VkCommandBuffer Renderer::BeginFrame()
{
	assertMsgReturnValue(!IsFrameStarted(), "Cannot call begin frame while is already in progress", VK_NULL_HANDLE);

	VkResult result = m_swapChain->AcquireNextImage(&m_currentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return VK_NULL_HANDLE;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		// handle in the future when the window get resize, so no error here!
		throw std::runtime_error("failed to acquire swap chain image!");
	}

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

void Renderer::EndFrame()
{
	assertMsgReturnVoid(IsFrameStarted(), "Cannot call end frame while the frame is not in progress");

	auto commandBuffer = GetCurrentCommandBuffer();

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	VkResult result = m_swapChain->SubmitCommandBuffers(&commandBuffer, &m_currentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.WasWindowResized())
	{
		m_window.ResetWindowResizedFlag();
		RecreateSwapChain();
	} 
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	m_isFrameStarted = false;

	m_currentFrameIndex = (m_currentFrameIndex + 1) % SwapChain::kMaxFramesInFlight;
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer _commandBuffer)
{
	assertMsgReturnVoid(IsFrameStarted(), "Cannot call BeginSwapChainRenderPass while the frame is not in progress");
	assertMsgReturnVoid(_commandBuffer == GetCurrentCommandBuffer(), "Cannot begin render pass on command buffer from a different frame");

	// we record the render pass to begin with
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_swapChain->GetRenderPass();

	// we need to say which frame buffer this render pass is writing in
	renderPassInfo.framebuffer = m_swapChain->GetFrameBuffer(m_currentImageIndex);

	// define the area where the shader loads and stores will take place
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapChain->GetSwapChainExtent();

	// clear values, which are the initial values of the frame buffer attachments
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };	// in the render pass, the attachment 0 is the color buffer
	clearValues[1].depthStencil = { 1.0f, 0 };			// in the render pass, the attachment 1 is the depth buffer

	renderPassInfo.clearValueCount = static_cast<uint32>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	// Now record this command buffer to begin this render pass
	// VK_SUBPASS_CONTENTS_INLINE is signaling that the subsequent render pass commands 
	// will be directly embedded in the primary command buffer itself and no secondary command buffer will be used
	// So cannot be mixed, or the render pass is only from primary or is only from secondary
	vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


	// The Viewport describe the transformation between the pipeline's output and target image
	// The viewport is in continuous range of (-1,-1) to (1,1), starting from top left going to bottom right. The middle is (0,0)
	// The framebuffer, the target image, start from (0,0) and go to (width, height), from top left to bottom right.
	// The Viewport is dealing with this transformation
	// So the Viewport tells to the pipeline how we want to transform the gl_Position values (the one in the vertex shader) to the output image
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapChain->GetSwapChainExtent().width);
	viewport.height = static_cast<float>(m_swapChain->GetSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor is like Viewport, but instead "squash" the result image (Viewport.width/height if changed keep the image but squashed), it will cut it.
	// Any pixels outside of the Scissor rectangle (offset + extent) will be discarded.
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapChain->GetSwapChainExtent();

	vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);
}

void Renderer::EndSwapChainRenderPass(VkCommandBuffer _commandBuffer)
{
	assertMsgReturnVoid(IsFrameStarted(), "Cannot call EndSwapChainRenderPass while the frame is not in progress");
	assertMsgReturnVoid(_commandBuffer == GetCurrentCommandBuffer(), "Cannot end render pass on command buffer from a different frame");

	vkCmdEndRenderPass(_commandBuffer);
}

VESPERENGINE_NAMESPACE_END
