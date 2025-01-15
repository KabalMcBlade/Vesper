// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\brdf_lut_generation_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/brdf_lut_generation_system.h"


VESPERENGINE_NAMESPACE_BEGIN

struct VESPERENGINE_ALIGN16 PushResolution
{
	glm::vec2 Resolution{ 512.0f, 512.0f };
};

BRDFLUTGenerationSystem::BRDFLUTGenerationSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass)
	: m_app{ _app }
	, CoreRenderSystem(_device)
{
	m_buffer = std::make_unique<Buffer>(m_device);

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushResolution);

	m_pushConstants.push_back(pushConstantRange);

	CreatePipelineLayout(std::vector<VkDescriptorSetLayout>{ });
	CreatePipeline(_renderPass);
}

BRDFLUTGenerationSystem::~BRDFLUTGenerationSystem()
{
	m_buffer->Destroy(m_quadVertexBufferComponent);
}

void BRDFLUTGenerationSystem::Generate(VkCommandBuffer _commandBuffer, uint32 _width, uint32 _height)
{
	const uint32 vertexCount = 3;
	const VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;
	const uint32 vertexSize = sizeof(Vertex);

	m_quadVertexBufferComponent = m_buffer->Create<VertexBufferComponent>(
		vertexSize,
		vertexCount,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	m_pipeline->Bind(_commandBuffer);

	const PushResolution resolution{{ _width, _height }};
	PushConstants(_commandBuffer, 0, &resolution);

	Bind(m_quadVertexBufferComponent, _commandBuffer);
	Draw(m_quadVertexBufferComponent, _commandBuffer);
}

void BRDFLUTGenerationSystem::CreatePipeline(VkRenderPass _renderPass)
{
	assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};

	Pipeline::DefaultPipelineConfiguration(pipelineConfig);

	pipelineConfig.RenderPass = _renderPass;
	pipelineConfig.PipelineLayout = m_pipelineLayout;

	pipelineConfig.DepthStencilInfo.depthTestEnable = VK_FALSE;
	pipelineConfig.DepthStencilInfo.depthWriteEnable = VK_FALSE;
	pipelineConfig.DepthStencilInfo.stencilTestEnable = VK_FALSE;

	pipelineConfig.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;

	pipelineConfig.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	pipelineConfig.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;

	m_pipeline = std::make_unique<Pipeline>(
		m_device,
		std::vector{ ShaderInfo{m_app.GetConfig().ShadersPath + "fullscreen.vert.spv", ShaderType::Vertex}, ShaderInfo{m_app.GetConfig().ShadersPath + "brdf_lut_shader.frag.spv", ShaderType::Fragment}, },
		pipelineConfig
	);
}

VESPERENGINE_NAMESPACE_END
