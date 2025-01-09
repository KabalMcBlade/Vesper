// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\master_render_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/master_render_system.h"

#include "Backend/swap_chain.h"


VESPERENGINE_NAMESPACE_BEGIN


// Scene Uniform Buffer Object
struct VESPERENGINE_ALIGN16 SceneUBO
{
	glm::mat4 ProjectionMatrix{ 1.0f };
	glm::mat4 ViewMatrix{ 1.0f };
	glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 0.3f };	// w is intensity
};

// Light Uniform Buffer Object
struct VESPERENGINE_ALIGN16 LightUBO
{
	glm::vec4 LightPos{ 0.0f, -0.25f, 0.0f, 0.0f };
	glm::vec4 LightColor{ 1.0f, 1.0f, 1.0f, 0.5f };
};


MasterRenderSystem::MasterRenderSystem(Device& _device)
	: CoreRenderSystem(_device)
{
	// IS ONLY FOR TESTING!!!
	// THIS IS HERE BECAUSE THERE IS ONE IN THE OPAQUE RENDERER AND IT IS IN NEED FOR COMMON PIPELINE LAYOUT!
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(glm::vec3);
	m_pushConstants.push_back(pushConstantRange);
	//////////////////////////////////////////////////////////////////////////

	// Start from here:
	m_buffer = std::make_unique<Buffer>(m_device);

	m_globalSetLayout = DescriptorSetLayout::Builder(m_device)
		.AddBinding(kGlobalBindingSceneIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(kGlobalBindingLightsIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build();

	CreatePipelineLayout(std::vector<VkDescriptorSetLayout>{ m_globalSetLayout->GetDescriptorSetLayout() });
}

void MasterRenderSystem::Initialize(DescriptorPool& _globalDescriptorPool)
{
	m_globalDescriptorSets.resize(SwapChain::kMaxFramesInFlight);

	m_globalSceneUboBuffers.resize(SwapChain::kMaxFramesInFlight);
	m_globalLightsUboBuffers.resize(SwapChain::kMaxFramesInFlight);

	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		m_globalSceneUboBuffers[i] = m_buffer->Create<BufferComponent>(
			sizeof(SceneUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, //VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			1,
			true
		);

		m_globalLightsUboBuffers[i] = m_buffer->Create<BufferComponent>(
			sizeof(LightUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, //VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			1,
			true
		);
	}

	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		auto sceneBufferInfo = m_buffer->GetDescriptorInfo(m_globalSceneUboBuffers[i]);
		auto lightBufferInfo = m_buffer->GetDescriptorInfo(m_globalLightsUboBuffers[i]);

		DescriptorWriter(*m_globalSetLayout, _globalDescriptorPool)
			.WriteBuffer(kGlobalBindingSceneIndex, &sceneBufferInfo)
			.WriteBuffer(kGlobalBindingLightsIndex, &lightBufferInfo)
			.Build(m_globalDescriptorSets[i]);
	}
}

void MasterRenderSystem::UpdateScene(const FrameInfo& _frameInfo, const CameraComponent& _cameraComponent)
{
	// check if is better in terms of performance to store them as class members and reusing it, or create per frame as here.
	SceneUBO sceneUBO;
	LightUBO lightsUBO;

	sceneUBO.ProjectionMatrix = _cameraComponent.ProjectionMatrix;
	sceneUBO.ViewMatrix = _cameraComponent.ViewMatrix;

	m_globalSceneUboBuffers[_frameInfo.FrameIndex].MappedMemory = &sceneUBO;
	m_buffer->WriteToBuffer(m_globalSceneUboBuffers[_frameInfo.FrameIndex]);

	m_globalLightsUboBuffers[_frameInfo.FrameIndex].MappedMemory = &lightsUBO;
	m_buffer->WriteToBuffer(m_globalLightsUboBuffers[_frameInfo.FrameIndex]);
}

void MasterRenderSystem::BindGlobalDescriptor(const FrameInfo& _frameInfo)
{
	// GLOBAL DESCRIPTOR: SCENE
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
}

void MasterRenderSystem::Cleanup()
{
	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		m_buffer->Destroy(m_globalSceneUboBuffers[i]);
		m_buffer->Destroy(m_globalLightsUboBuffers[i]);
	}
}

VESPERENGINE_NAMESPACE_END
