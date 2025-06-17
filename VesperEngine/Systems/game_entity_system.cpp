// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\game_entity_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/game_entity_system.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/camera_components.h"
#include "Components/pipeline_components.h"
#include "Components/light_components.h"

#include "App/vesper_app.h"

#include "ECS/ECS/ecs.h"


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
		m_app.GetComponentManager().AddComponent<DynamicOffsetComponent>(entity);
		break;
	case EntityType::DirectionalLight:
		m_app.GetComponentManager().AddComponent<DirectionalLightComponent>(entity);
		break;
	case EntityType::PointLight:
		m_app.GetComponentManager().AddComponent<PointLightComponent>(entity);
		m_app.GetComponentManager().AddComponent<TransformComponent>(entity);
		break;
	case EntityType::SpotLight:
		m_app.GetComponentManager().AddComponent<SpotLightComponent>(entity);
		m_app.GetComponentManager().AddComponent<TransformComponent>(entity);
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

	if (m_app.GetComponentManager().HasComponents<DynamicOffsetComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<DynamicOffsetComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<DirectionalLightComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<DirectionalLightComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<PointLightComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<PointLightComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<SpotLightComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<SpotLightComponent>(_entity);
	}

	m_app.GetEntityManager().DestroyEntity(_entity);
}

void GameEntitySystem::DestroyGameEntities() const
{
	// IMPORTANT: 
	// Here need the collect, because we destroy the entity, hence the iterator is changed during iteration, which cause an assert of missing entity.
	// So in this case use ecs::EntityCollector::CollectEntitiesWithAll and not the ecs::IterateEntitiesWithAll.

	// destroy just whatever defined by the enum EntityType, so either TransformComponent or CameraComponent
	std::vector<ecs::Entity> entities = ecs::EntityCollector::CollectEntitiesWithAny<TransformComponent, CameraTransformComponent>(m_app.GetEntityManager(), m_app.GetComponentManager());
	for (const ecs::Entity entity : entities)
	{
		DestroyGameEntity(entity);
	}
	entities.clear();
}


VESPERENGINE_NAMESPACE_END
