#include "pch.h"
#include "game_entity_system.h"


VESPERENGINE_NAMESPACE_BEGIN

ecs::Entity GameEntitySystem::CreateGameEntity(EntityType _type) const
{
	ecs::Entity entity = ecs::EntityManager::CreateEntity();

	switch(_type)
	{
	case EntityType::Camera:
		ecs::ComponentManager::AddComponent<CameraTransformComponent>(entity);
		ecs::ComponentManager::AddComponent<CameraComponent>(entity);
		break;
    case EntityType::Object:
		ecs::ComponentManager::AddComponent<TransformComponent>(entity);
		break;
	default:
		// Pure
		break;
	}

	return entity;
}

void GameEntitySystem::DestroyGameEntity(const ecs::Entity _entity) const
{
	if (ecs::ComponentManager::HasComponents<CameraComponent>(_entity))
	{
		ecs::ComponentManager::RemoveComponent<CameraComponent>(_entity);
	}

	if (ecs::ComponentManager::HasComponents<CameraTransformComponent>(_entity))
	{
		ecs::ComponentManager::RemoveComponent<CameraTransformComponent>(_entity);
	}

	if (ecs::ComponentManager::HasComponents<TransformComponent>(_entity))
	{
		ecs::ComponentManager::RemoveComponent<TransformComponent>(_entity);
	}

	ecs::EntityManager::DestroyEntity(_entity);
}

void GameEntitySystem::DestroyGameEntities() const
{
	// IMPORTANT: 
	// Here need the collect, because we destroy the entity, hence the iterator is changed during iteration, which cause an assert of missing entity.
	// So in this case use ecs::EntityCollector::CollectEntitiesWithAll and not the ecs::IterateEntitiesWithAll.

	// destroy just whatever defined by the enum EntityType, so either TransformComponent or CameraComponent
	std::vector<ecs::Entity> entities;
	ecs::EntityCollector::CollectEntitiesWithAny<TransformComponent, CameraTransformComponent>(entities);
	for (const ecs::Entity entity : entities)
	{
		DestroyGameEntity(entity);
	}
	entities.clear();
}


VESPERENGINE_NAMESPACE_END
