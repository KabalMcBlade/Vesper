// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\entity_handler_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/entity_handler_system.h"

#include "Backend/buffer.h"
#include "Backend/swap_chain.h"

#include "Utility/logger.h"

#include "ECS/ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

EntityHandlerSystem::EntityHandlerSystem(VesperApp& _app, Device& _device)
	: m_app(_app)
	, m_device(_device)
{
	m_buffer = std::make_unique<Buffer>(m_device);

	const uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);
	m_alignedSizeUBO = m_buffer->GetAlignment<uint32>(sizeof(EntityUBO), minUboAlignment);

	m_entitySetLayout = DescriptorSetLayout::Builder(m_device)
		.AddBinding(kEntityBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT)
		.Build();
}

void EntityHandlerSystem::Initialize(DescriptorPool& _globalDescriptorPool)
{
	m_entityUboBuffers.resize(SwapChain::kMaxFramesInFlight);
	m_entityDescriptorSets.resize(SwapChain::kMaxFramesInFlight);

	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		m_entityUboBuffers[i] = m_buffer->Create<BufferComponent>(
			sizeof(EntityUBO),
			GetEntityCount(),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, //VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			GetAlignedSizeUBO(),
			true
		);
	}

	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		auto objectBufferInfo = m_buffer->GetDescriptorInfo(m_entityUboBuffers[i]);

		DescriptorWriter(*m_entitySetLayout, _globalDescriptorPool)
			.WriteBuffer(kEntityBindingIndex, &objectBufferInfo)
			.Build(m_entityDescriptorSets[i]);
	}
}

void EntityHandlerSystem::UpdateEntities(const FrameInfo& _frameInfo)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto gameEntity : ecs::IterateEntitiesWithAll<DynamicOffsetComponent, RenderComponent>(entityManager, componentManager))
	{
		const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(gameEntity);
		const RenderComponent& renderComponent = componentManager.GetComponent<RenderComponent>(gameEntity);

		EntityUBO entityUBO{ renderComponent.ModelMatrix };

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
}

void EntityHandlerSystem::RegisterEntity(ecs::Entity _entity) const
{
	if (!m_app.GetComponentManager().HasComponents<RenderComponent>(_entity))
	{
		m_app.GetComponentManager().AddComponent<RenderComponent>(_entity);
	}

	if (!m_app.GetComponentManager().HasComponents<DynamicOffsetComponent>(_entity))
	{
		m_app.GetComponentManager().AddComponent<DynamicOffsetComponent>(_entity);
	}
	
	DynamicOffsetComponent& dynamicUniformBufferComponent = m_app.GetComponentManager().GetComponent<DynamicOffsetComponent>(_entity);
	dynamicUniformBufferComponent.DynamicOffsetIndex = m_internalCounter;
	dynamicUniformBufferComponent.DynamicOffset = m_internalCounter * m_alignedSizeUBO;

	++m_internalCounter;
}

void EntityHandlerSystem::UnregisterEntity(ecs::Entity _entity) const
{
	if (m_app.GetComponentManager().HasComponents<RenderComponent, DynamicOffsetComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<RenderComponent>(_entity);
		m_app.GetComponentManager().RemoveComponent<DynamicOffsetComponent>(_entity);
	}
}

void EntityHandlerSystem::UnregisterEntities() const
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto entity : ecs::IterateEntitiesWithAll<RenderComponent, DynamicOffsetComponent>(entityManager, componentManager))
	{
		UnregisterEntity(entity);
	}
}

VESPERENGINE_NAMESPACE_END
