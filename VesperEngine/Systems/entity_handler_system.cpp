// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\entity_handler_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/uniform_buffer.h"			// NOTE: dependency forced, compiler need to know the size of the UBOs used here first
#include "Systems/entity_handler_system.h"

#include "Backend/buffer.h"
#include "Backend/swap_chain.h"
#include "Backend/frame_info.h"
#include "Backend/device.h"
#include "Backend/renderer.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"

#include "App/vesper_app.h"

#include "ECS/ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

EntityHandlerSystem::EntityHandlerSystem(VesperApp& _app, Device& _device, Renderer& _renderer)
	: m_app(_app)
	, m_device(_device)
	, m_renderer(_renderer)
{
	m_buffer = std::make_unique<Buffer>(m_device);

	const uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);
	m_alignedSizeUBO = m_buffer->GetAlignment<uint32>(sizeof(EntityUBO), minUboAlignment);

	m_entitySetLayout = DescriptorSetLayout::Builder(m_device)
		.AddBinding(kEntityBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT)
		.Build();
}

void EntityHandlerSystem::Initialize()
{
	m_entityUboBuffers.resize(SwapChain::kMaxFramesInFlight);
	m_entityDescriptorSets.resize(SwapChain::kMaxFramesInFlight);

	const uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);
	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		m_entityUboBuffers[i] = m_buffer->Create<BufferComponent>(
			sizeof(EntityUBO),
			GetEntityCount(),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, //VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			minUboAlignment,
			true
		);
	}

	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		auto objectBufferInfo = m_buffer->GetDescriptorInfo(m_entityUboBuffers[i]);

		DescriptorWriter(*m_entitySetLayout, *m_renderer.GetDescriptorPool())
			.WriteBuffer(kEntityBindingIndex, &objectBufferInfo)
			.Build(m_entityDescriptorSets[i]);
	}
}

void EntityHandlerSystem::UpdateEntities(const FrameInfo& _frameInfo)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto gameEntity : ecs::IterateEntitiesWithAll<DynamicOffsetComponent, UpdateComponent>(entityManager, componentManager))
	{
		const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(gameEntity);
		const UpdateComponent& updateComponent = componentManager.GetComponent<UpdateComponent>(gameEntity);

		glm::vec4 morphWeights0(0.0f);
		glm::vec4 morphWeights1(0.0f);
		int32 morphCount = 0;
		if (componentManager.HasComponents<MorphWeightsComponent>(gameEntity))
		{
			const MorphWeightsComponent& comp = componentManager.GetComponent<MorphWeightsComponent>(gameEntity);
			morphWeights0 = comp.Weights[0];
			morphWeights1 = comp.Weights[1];
			morphCount = static_cast<int32>(comp.Count);
		}

		EntityUBO entityUBO{};
		entityUBO.ModelMatrix = updateComponent.ModelMatrix;
		entityUBO.MorphWeights0 = morphWeights0;
		entityUBO.MorphWeights1 = morphWeights1;
		entityUBO.MorphTargetCount = morphCount;

		m_entityUboBuffers[_frameInfo.FrameIndex].MappedMemory = &entityUBO;
		m_buffer->WriteToIndex(m_entityUboBuffers[_frameInfo.FrameIndex], dynamicOffsetComponent.DynamicOffsetIndex);
	}
}

void EntityHandlerSystem::Cleanup()
{
	UnregisterEntities();

	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		m_buffer->Destroy(m_entityUboBuffers[i]);
	}

	m_internalCounter = 0;
}

void EntityHandlerSystem::RegisterRenderableEntity(ecs::Entity _entity) const
{
	DynamicOffsetComponent& dynamicUniformBufferComponent = m_app.GetComponentManager().GetComponent<DynamicOffsetComponent>(_entity);
	dynamicUniformBufferComponent.DynamicOffsetIndex = m_internalCounter;
	dynamicUniformBufferComponent.DynamicOffset = m_internalCounter * m_alignedSizeUBO;

	++m_internalCounter;
}

void EntityHandlerSystem::UnregisterEntities() const
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto entity : ecs::IterateEntitiesWithAll<DynamicOffsetComponent>(entityManager, componentManager))
	{
		DynamicOffsetComponent& dynamicUniformBufferComponent = m_app.GetComponentManager().GetComponent<DynamicOffsetComponent>(entity);
		dynamicUniformBufferComponent.DynamicOffsetIndex = 0u;
		dynamicUniformBufferComponent.DynamicOffset = 0u;
	}
}

VESPERENGINE_NAMESPACE_END
