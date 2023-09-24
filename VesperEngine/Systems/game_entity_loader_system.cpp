#include "pch.h"
#include "game_entity_loader_system.h"

#include "Core/memory_copy.h"
#include "Backend/device.h"

#include "Components/core_components.h"

#include "ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

GameEntityLoaderSystem::GameEntityLoaderSystem(Device& _device)
	: m_device{ _device }
{
}

GameEntityLoaderSystem::~GameEntityLoaderSystem()
{
}

ecs::Entity GameEntityLoaderSystem::CreateGameEntity()
{
	ecs::Entity entity = ecs::EntityManager::CreateEntity();

	ecs::ComponentManager::AddComponent<RenderComponent>(entity);
	ecs::ComponentManager::AddComponent<TransformComponent>(entity);
	ecs::ComponentManager::AddComponent<MaterialComponent>(entity);

	return entity;
}

void GameEntityLoaderSystem::DestroyGameEntity(const ecs::Entity _entity)
{
	ecs::ComponentManager::RemoveComponent<MaterialComponent>(_entity);
	ecs::ComponentManager::RemoveComponent<TransformComponent>(_entity);
	ecs::ComponentManager::RemoveComponent<RenderComponent>(_entity);

	ecs::EntityManager::DestroyEntity(_entity);
}

void GameEntityLoaderSystem::DestroyGameEntities()
{
	// IMPORTANT: 
	// Here need the collect, because we destroy the entity, hence the iterator is changed during iteration, which cause an assert of missing entity.
	// So in this case use ecs::EntityCollector::CollectEntitiesWithAll and not the ecs::IterateEntitiesWithAll.

	std::vector<ecs::Entity> entities;
	ecs::EntityCollector::CollectEntitiesWithAll<RenderComponent, TransformComponent, MaterialComponent>(entities);
	for (const ecs::Entity entity : entities)
	{
		DestroyGameEntity(entity);
	}
	entities.clear();
}

void GameEntityLoaderSystem::LoadGameEntity(ecs::Entity _entity, const std::vector<Vertex> _vertices)
{
	const bool bHasComponent = ecs::ComponentManager::HasComponents<RenderComponent>(_entity);
	if (!bHasComponent)
	{
		return;
	}

	RenderComponent& renderComponent = ecs::ComponentManager::GetComponent<RenderComponent>(_entity);

	renderComponent.VertexCount = static_cast<uint32>(_vertices.size());

	assertMsgReturnVoid(renderComponent.VertexCount >= 3, "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(_vertices[0]) * renderComponent.VertexCount;

	m_device.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		renderComponent.VertexBuffer,
		renderComponent.VertexBufferMemory
	);

	void* data;
	vmaMapMemory(m_device.GetAllocator(), renderComponent.VertexBufferMemory, &data);

	MemCpy(data, _vertices.data(), static_cast<std::size_t>(bufferSize));

	vmaUnmapMemory(m_device.GetAllocator(), renderComponent.VertexBufferMemory);
}

void GameEntityLoaderSystem::UnloadGameEntity(ecs::Entity _entity)
{
	const bool bHasComponent = ecs::ComponentManager::HasComponents<RenderComponent>(_entity);
	if (!bHasComponent)
	{
		return;
	}

	RenderComponent& renderComponent = ecs::ComponentManager::GetComponent<RenderComponent>(_entity);

	vmaDestroyBuffer(m_device.GetAllocator(), renderComponent.VertexBuffer, renderComponent.VertexBufferMemory);
}

void GameEntityLoaderSystem::LoadGameEntities(const std::vector<Vertex> _vertices)
{
	for (auto iterator : ecs::IterateEntitiesWithAll<RenderComponent, TransformComponent, MaterialComponent>())
	{
		LoadGameEntity(iterator, _vertices);
	}
}

void GameEntityLoaderSystem::UnloadGameEntities()
{
	for (auto iterator : ecs::IterateEntitiesWithAll<RenderComponent, TransformComponent, MaterialComponent>())
	{
		UnloadGameEntity(iterator);
	}
}

VESPERENGINE_NAMESPACE_END
