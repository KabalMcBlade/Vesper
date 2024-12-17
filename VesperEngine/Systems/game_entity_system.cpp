#include "pch.h"
#include "game_entity_system.h"

#include "App/vesper_app.h"

VESPERENGINE_NAMESPACE_BEGIN

GameEntitySystem::GameEntitySystem(VesperApp& _app)
	: m_app(_app)
{
}

ecs::Entity GameEntitySystem::CreateGameEntity(EntityType _type) const
{
	ecs::Entity entity = m_app.GetEntityManager().CreateEntity();

	switch(_type)
	{
	case EntityType::Camera:
		m_app.GetComponentManager().AddComponent<CameraTransformComponent>(entity);
		m_app.GetComponentManager().AddComponent<CameraComponent>(entity);
		break;
    case EntityType::Object:
		m_app.GetComponentManager().AddComponent<TransformComponent>(entity);
		break;
	case EntityType::Renderable:
		m_app.GetComponentManager().AddComponent<TransformComponent>(entity);
		m_app.GetComponentManager().AddComponent<RenderComponent>(entity);
		break;
	default:
		// Pure
		break;
	}

	return entity;
}

void GameEntitySystem::DestroyGameEntity(const ecs::Entity _entity) const
{
	if (m_app.GetComponentManager().HasComponents<CameraComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<CameraComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<CameraTransformComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<CameraTransformComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<TransformComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<TransformComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<RenderComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<RenderComponent>(_entity);
	}

	m_app.GetEntityManager().DestroyEntity(_entity);
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
