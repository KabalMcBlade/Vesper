#include "pch.h"

#include "Systems/base_render_system.h"


VESPERENGINE_NAMESPACE_BEGIN

BaseRenderSystem::BaseRenderSystem(Device& _device)
	: m_device{ _device }
{
}

BaseRenderSystem::~BaseRenderSystem()
{
	vkDestroyPipelineLayout(m_device.GetDevice(), m_pipelineLayout, nullptr);
}

void BaseRenderSystem::Update(FrameInfo& _frameInfo)
{
	UpdateFrame(_frameInfo);
}

void BaseRenderSystem::Render(FrameInfo& _frameInfo)
{
	m_pipeline->Bind(_frameInfo.CommandBuffer);

	vkCmdBindDescriptorSets(
		_frameInfo.CommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
		m_pipelineLayout,
		0,
		1,
		&_frameInfo.GlobalDescriptorSet,
		0,
		nullptr
	);

	RenderFrame(_frameInfo);
}

void BaseRenderSystem::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	// This is used to pass other than vertex data to the vertex and fragment shaders.
	// This can include textures and uniform buffer objects (UBO)
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32>(_descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = _descriptorSetLayouts.data();

	// Push constants are a efficient way to send a small amount of data to the shader programs
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32>(m_pushConstants.size());
	pipelineLayoutInfo.pPushConstantRanges = m_pushConstants.data();

	if (vkCreatePipelineLayout(m_device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void BaseRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
	assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::DefaultPipelineConfiguration(pipelineConfig);

	// The render pass describes the structure and format of the frame buffer objects (FBOs) and their attachments
	// Right now we have in location 0 the color buffer and in location 1 the depth buffer, check SwapChain::CreateRenderPass() -> attachments
	// So we need to inform the shaders about these locations. If the render pass change, shaders must reflect it.
	pipelineConfig.RenderPass = _renderPass;
	pipelineConfig.PipelineLayout = m_pipelineLayout;

	SetupePipeline(pipelineConfig);
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

void BaseRenderSystem::Bind(VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	VkBuffer buffers[] = { _vertexBufferComponent.Buffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, buffers, offsets);
}

void BaseRenderSystem::Bind(VertexBufferComponent& _vertexBufferComponent, IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	Bind(_vertexBufferComponent, _commandBuffer);
	vkCmdBindIndexBuffer(_commandBuffer, _indexBufferComponent.Buffer, 0, VK_INDEX_TYPE_UINT32);
}

void BaseRenderSystem::Draw(VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	vkCmdDraw(_commandBuffer, _vertexBufferComponent.Count, 1, 0, 0);
}

void BaseRenderSystem::Draw(IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const
{
	vkCmdDrawIndexed(_commandBuffer, _indexBufferComponent.Count, 1, 0, 0, 0);
}

VESPERENGINE_NAMESPACE_END
