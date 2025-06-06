// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\master_render_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/master_render_system.h"
#include "Systems/uniform_buffer.h"

#include "Backend/swap_chain.h"
#include "Backend/buffer.h"
#include "Backend/pipeline.h"
#include "Backend/frame_info.h"
#include "Backend/renderer.h"

#include "Components/graphics_components.h"
#include "Components/camera_components.h"

#include "Systems/texture_system.h"
#include "Systems/material_system.h"


VESPERENGINE_NAMESPACE_BEGIN

MasterRenderSystem::MasterRenderSystem(Device& _device, Renderer& _renderer)
	: BaseRenderSystem(_device)
	, m_renderer(_renderer)
{


	// Start from here:
	m_buffer = std::make_unique<Buffer>(m_device);

	// will be valid only if device support it
	if (m_device.IsBindlessResourcesSupported())
	{
		m_globalSetLayout = DescriptorSetLayout::Builder(m_device)
			.AddBinding(kGlobalBindingSceneIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddBinding(kGlobalBindingLightsIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build();

		m_bindlesslSetLayout = DescriptorSetLayout::Builder(m_device)
			.SetFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)
			.AddBinding(kBindlessBindingTexturesIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, kMaxBindlessTextures, kMaxBindlessTextures >> 3)
			.AddBinding(kBindlessBindingBuffersIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, kMaxBindlessBuffers, kMaxBindlessBuffers >> 3)
			.Build();


		CreatePipelineLayout(std::vector<VkDescriptorSetLayout>{ m_globalSetLayout->GetDescriptorSetLayout(), m_bindlesslSetLayout->GetDescriptorSetLayout() });
	}
	else
	{
		m_globalSetLayout = DescriptorSetLayout::Builder(m_device)
			.AddBinding(kGlobalBindingSceneIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddBinding(kGlobalBindingLightsIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build();

		CreatePipelineLayout(std::vector<VkDescriptorSetLayout>{ m_globalSetLayout->GetDescriptorSetLayout() });
	}
}

void MasterRenderSystem::Initialize(TextureSystem& _textureSystem, MaterialSystem& _materialSystem)
{
	m_globalDescriptorSets.resize(SwapChain::kMaxFramesInFlight);
	m_bindlessBindingDescriptorSets.resize(SwapChain::kMaxFramesInFlight);

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

		DescriptorWriter(*m_globalSetLayout, *m_renderer.GetDescriptorPool())
			.WriteBuffer(kGlobalBindingSceneIndex, &sceneBufferInfo)
			.WriteBuffer(kGlobalBindingLightsIndex, &lightBufferInfo)
			.Build(m_globalDescriptorSets[i]);
	}

	if (m_device.IsBindlessResourcesSupported())
	{
		const auto& textures = _textureSystem.GetTextures();
		const uint32 textureCount = static_cast<uint32>(textures.size());
		std::vector<VkDescriptorImageInfo> imageInfos(textureCount);
		for (size_t i = 0; i < textureCount; ++i)
		{
			imageInfos[i].imageView = textures[i]->ImageView;
			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfos[i].sampler = textures[i]->Sampler;
		}

		const auto& materials = _materialSystem.GetMaterials();
		const uint32 materialCount = static_cast<uint32>(materials.size());
		std::vector<VkDescriptorBufferInfo> bufferInfos(materialCount);
		for (size_t i = 0; i < materialCount; ++i) 
		{
			bufferInfos[i] = m_buffer->GetDescriptorInfo(materials[i]->UniformBuffer);
		}
		
		for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
		{
			DescriptorWriter(*m_bindlesslSetLayout, *m_renderer.GetDescriptorPool())
				.WriteImage(kBindlessBindingTexturesIndex, imageInfos.data(), textureCount)
				.WriteBuffer(kBindlessBindingBuffersIndex, bufferInfos.data(), materialCount)
				.Build(m_bindlessBindingDescriptorSets[i]);
		}
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
		&m_globalDescriptorSets[_frameInfo.FrameIndex],
		//&_frameInfo.GlobalDescriptorSet,
		0,
		nullptr
	);

	if (m_device.IsBindlessResourcesSupported())
	{
		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
			m_pipelineLayout,
			1,
			1,
			&m_bindlessBindingDescriptorSets[_frameInfo.FrameIndex],
			0,
			nullptr
		);
	}
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
