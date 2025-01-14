// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\opaque_render_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "opaque_render_system.h"

#include <array>
#include <stdexcept>
#include <iostream>

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "gtx/quaternion.hpp"

#include "Backend/swap_chain.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/pipeline_components.h"

#include "Systems/uniform_buffer.h"

#include "Utility/logger.h"

#include "ECS/ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

// TEST PUSH CONSTAT!
struct ColorTintPushConstantData
{
	glm::vec3 ColorTint{ 1.0f };
};

OpaqueRenderSystem::OpaqueRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
	VkDescriptorSetLayout _globalDescriptorSetLayout,
	VkDescriptorSetLayout _entityDescriptorSetLayout,
	VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout)
	: CoreRenderSystem{ _device }
	, m_app(_app)
	, m_renderer(_renderer)
{
	m_buffer = std::make_unique<Buffer>(m_device);

	m_app.GetComponentManager().RegisterComponent<ColorTintPushConstantData>();

	if (m_device.IsBindlessResourcesSupported())
	{
		// in this case is just for the indices!
		m_materialSetLayout = DescriptorSetLayout::Builder(_device)
			.AddBinding(kPhongUniformBufferOnlyBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build();
	}
	else
	{
		m_materialSetLayout = DescriptorSetLayout::Builder(_device)
			.AddBinding(kPhongAmbientTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddBinding(kPhongDiffuseTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddBinding(kPhongSpecularTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddBinding(kPhongNormalTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddBinding(kPhongUniformBufferBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build();
	}

	// TEST PUSH CONSTANT!
	// REMEMBER THAT TO TEST THIS WITH THIS PIPELINE AND LAYOUT ALSO THE MASTER RENDERER NEED TO HAVE!
	// SINCE IS A TEST, REMEMBER TO REMOVE BOTH HERE AND IN MASTER RENDERER EVENTUALLY!
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(ColorTintPushConstantData);

	m_pushConstants.push_back(pushConstantRange);

	// set 0: global descriptor set layout
	// set 1: bindless textures and buffer descriptor set layout
	// set 2: entity descriptor set layout
	// set 3: material descriptor set layout
	// OR
	// set 0: global descriptor set layout
	// set 1: entity descriptor set layout
	// set 2: material descriptor set layout
	if (m_device.IsBindlessResourcesSupported())
	{
		m_entitySetIndex = 2;	// normally is 1
		m_materialSetIndex = 3;	// normally is 2

		CreatePipelineLayout(std::vector<VkDescriptorSetLayout>
			{ _globalDescriptorSetLayout, _bindlessBindingDescriptorSetLayout, _entityDescriptorSetLayout, m_materialSetLayout->GetDescriptorSetLayout() }
		);
	}
	else
	{
		CreatePipelineLayout(std::vector<VkDescriptorSetLayout>
			{ _globalDescriptorSetLayout, _entityDescriptorSetLayout, m_materialSetLayout->GetDescriptorSetLayout() }
		);
	}

	CreatePipeline(m_renderer.GetSwapChainRenderPass());
}

OpaqueRenderSystem::~OpaqueRenderSystem()
{
	m_app.GetComponentManager().UnregisterComponent<ColorTintPushConstantData>();
}

void OpaqueRenderSystem::MaterialBinding()
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto gameEntity : ecs::IterateEntitiesWithAll<PhongMaterialComponent>(entityManager, componentManager))
	{
		PhongMaterialComponent& materialComponent = m_app.GetComponentManager().GetComponent<PhongMaterialComponent>(gameEntity);
		materialComponent.BoundDescriptorSet.resize(SwapChain::kMaxFramesInFlight);

		if (m_device.IsBindlessResourcesSupported())
		{
			for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
			{
				m_bindlessBindingMaterialIndexUbos.emplace_back(m_buffer->Create<BufferComponent>(
					sizeof(PhongBindlessMaterialIndexUBO),
					1,
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
					VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
					/*minUboAlignment*/1,
					true
				));

				BufferComponent& currentBuffer = m_bindlessBindingMaterialIndexUbos.back();

				PhongBindlessMaterialIndexUBO phongIndexUBO;
				phongIndexUBO.MaterialIndex = materialComponent.Index;

				currentBuffer.MappedMemory = &phongIndexUBO;
				m_buffer->WriteToBuffer(currentBuffer);

				VkDescriptorBufferInfo bufferInfo;
				bufferInfo.buffer = currentBuffer.Buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = currentBuffer.AlignedSize;

 				DescriptorWriter(*m_materialSetLayout, *m_renderer.GetDescriptorPool())
 					.WriteBuffer(kPhongUniformBufferOnlyBindingIndex, &bufferInfo)
 					.Build(materialComponent.BoundDescriptorSet[i]);
			}
		}
		else
		{
			for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
			{
				DescriptorWriter(*m_materialSetLayout, *m_renderer.GetDescriptorPool())
					.WriteImage(kPhongAmbientTextureBindingIndex, &materialComponent.AmbientImageInfo)
					.WriteImage(kPhongDiffuseTextureBindingIndex, &materialComponent.DiffuseImageInfo)
					.WriteImage(kPhongSpecularTextureBindingIndex, &materialComponent.SpecularImageInfo)
					.WriteImage(kPhongNormalTextureBindingIndex, &materialComponent.NormalImageInfo)
					.WriteBuffer(kPhongUniformBufferBindingIndex, &materialComponent.UniformBufferInfo)
					.Build(materialComponent.BoundDescriptorSet[i]);
			}
		}
	}

	// TEST PUSH CONSTANT ONLY!
	// I add this here, because I do not want actually this in the opaque pipeline, but is for test only! 
	for (auto gameEntity : ecs::IterateEntitiesWithAll<PipelineOpaqueComponent>(entityManager, componentManager))
	{
		componentManager.AddComponent<ColorTintPushConstantData>(gameEntity);
	}
}

void OpaqueRenderSystem::Update(const FrameInfo& _frameInfo)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto gameEntity : ecs::IterateEntitiesWithAll<RenderComponent, TransformComponent, PipelineOpaqueComponent>(entityManager, componentManager))
	{
		TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);
		RenderComponent& renderComponent = componentManager.GetComponent<RenderComponent>(gameEntity);

		renderComponent.ModelMatrix = glm::translate(glm::mat4{ 1.0f }, transformComponent.Position);
		renderComponent.ModelMatrix = renderComponent.ModelMatrix * glm::toMat4(transformComponent.Rotation);
		renderComponent.ModelMatrix = glm::scale(renderComponent.ModelMatrix, transformComponent.Scale);

		ColorTintPushConstantData& pushComponent = componentManager.GetComponent<ColorTintPushConstantData>(gameEntity);

		// Use a sine wave to create smooth transitions for R, G, and B
		static const float speed = 1.0f;
		static float frameTimeUpdated = 0.0f;
		const float r = 0.5f * (std::sin(speed * frameTimeUpdated) + 1.0f); // Oscillates between 0 and 1
		const float g = 0.5f * (std::sin(speed * frameTimeUpdated + glm::pi<float>() / 3.0f) + 1.0f); // Offset by 120 degrees
		const float b = 0.5f * (std::sin(speed * frameTimeUpdated + 2.0f * glm::pi<float>() / 3.0f) + 1.0f); // Offset by 240 degrees
		frameTimeUpdated += _frameInfo.FrameTime;
		pushComponent.ColorTint = glm::vec3(r, g, b);
	}
}

void OpaqueRenderSystem::Render(const FrameInfo& _frameInfo)
{
	// this bind only the opaque pipeline
	m_opaquePipeline->Bind(_frameInfo.CommandBuffer);

	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	// 1. Render whatever has vertex buffers and index buffer
	auto entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<PhongMaterialComponent, PipelineOpaqueComponent, DynamicOffsetComponent, VertexBufferComponent, IndexBufferComponent, ColorTintPushConstantData>(entityManager, componentManager, &PhongMaterialComponent::Index);

	for (const auto& [key, entities] : entitiesGroupedAndCollected)
	{
		// From the first one and only, we can the material and we bind it.
		const PhongMaterialComponent& phongMaterialComponent = componentManager.GetComponent<PhongMaterialComponent>(entities[0]);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			m_materialSetIndex,
			1,
			&phongMaterialComponent.BoundDescriptorSet[_frameInfo.FrameIndex],
			0,
			nullptr
		);

		for (const auto& entityCollected : entities)
		{
			const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(entityCollected);
			const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(entityCollected);
			const IndexBufferComponent& indexBufferComponent = componentManager.GetComponent<IndexBufferComponent>(entityCollected);
			const ColorTintPushConstantData& pushComponent = componentManager.GetComponent<ColorTintPushConstantData>(entityCollected);

			vkCmdBindDescriptorSets(
				_frameInfo.CommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
				m_pipelineLayout,
				m_entitySetIndex,
				1,
				&_frameInfo.EntityDescriptorSet,
				1,
				&dynamicOffsetComponent.DynamicOffset
			);

			PushConstants(_frameInfo.CommandBuffer, 0, &pushComponent);
			Bind(vertexBufferComponent, indexBufferComponent, _frameInfo.CommandBuffer);
			Draw(indexBufferComponent, _frameInfo.CommandBuffer);
		}
	}

	entitiesGroupedAndCollected.clear();



	// 2. Render only entities having Vertex buffers only
	entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<PhongMaterialComponent, DynamicOffsetComponent, VertexBufferComponent, NotIndexBufferComponent, ColorTintPushConstantData>(entityManager, componentManager, &PhongMaterialComponent::Index);

	for (const auto& [key, entities] : entitiesGroupedAndCollected)
	{
		// From the first one and only, we can the material and we bind it.
		const PhongMaterialComponent& phongMaterialComponent = componentManager.GetComponent<PhongMaterialComponent>(entities[0]);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			m_materialSetIndex,
			1,
			&phongMaterialComponent.BoundDescriptorSet[_frameInfo.FrameIndex],
			0,
			nullptr
		);

		for (const auto& entityCollected : entities)
		{
			const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(entityCollected);
			const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(entityCollected);
			const ColorTintPushConstantData& pushComponent = componentManager.GetComponent<ColorTintPushConstantData>(entityCollected);

			vkCmdBindDescriptorSets(
				_frameInfo.CommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
				m_pipelineLayout,
				m_entitySetIndex,
				1,
				&_frameInfo.EntityDescriptorSet,
				1,
				&dynamicOffsetComponent.DynamicOffset
			);

			PushConstants(_frameInfo.CommandBuffer, 0, &pushComponent);
			Bind(vertexBufferComponent, _frameInfo.CommandBuffer);
			Draw(vertexBufferComponent, _frameInfo.CommandBuffer);
		}
	}

	entitiesGroupedAndCollected.clear();
}

void OpaqueRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
	assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};

	Pipeline::OpaquePipelineConfiguration(pipelineConfig);

	pipelineConfig.RenderPass = _renderPass;
	pipelineConfig.PipelineLayout = m_pipelineLayout;


	const std::string vertexShaderFilepath = m_device.IsBindlessResourcesSupported()
		? m_app.GetConfig().ShadersPath + "opaque_shader_bindless1.vert.spv"
		: m_app.GetConfig().ShadersPath + "opaque_shader_bindless0.vert.spv";

	ShaderInfo vertexShader(
		vertexShaderFilepath,
		ShaderType::Vertex
	);


	const std::string fragmentShaderFilepath = m_device.IsBindlessResourcesSupported() 
		? m_app.GetConfig().ShadersPath + "opaque_shader_bindless1.frag.spv"
		: m_app.GetConfig().ShadersPath + "opaque_shader_bindless0.frag.spv";

	ShaderInfo fragmentShader(
		fragmentShaderFilepath,
		ShaderType::Fragment
	);

	fragmentShader.AddSpecializationConstant(0, 2.0f);


	m_opaquePipeline = std::make_unique<Pipeline>(
		m_device,
		std::vector{
			vertexShader,
			fragmentShader,
		},
		pipelineConfig
		);
}

void OpaqueRenderSystem::Cleanup()
{
	for (int32 i = 0; i < m_bindlessBindingMaterialIndexUbos.size(); ++i)
	{
		m_buffer->Destroy(m_bindlessBindingMaterialIndexUbos[i]);
	}
}

VESPERENGINE_NAMESPACE_END
