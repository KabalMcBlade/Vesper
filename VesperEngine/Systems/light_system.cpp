// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\light_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/light_system.h"

#include "Components/light_components.h"
#include "Components/object_components.h"

#include "Systems/game_entity_system.h"
#include "Systems/uniform_buffer.h"

#include "App/vesper_app.h"

#include "ECS/ECS/ecs.h"

VESPERENGINE_NAMESPACE_BEGIN

LightSystem::LightSystem(VesperApp& _app, GameEntitySystem& _gameEntitySystem)
    : m_app(_app)
    , m_gameEntitySystem(_gameEntitySystem)
{
}

ecs::Entity LightSystem::CreateDirectionalLight(const glm::vec3& _direction, const glm::vec3& _color, float _intensity) const
{
    ecs::Entity entity = m_gameEntitySystem.CreateGameEntity(EntityType::DirectionalLight);
    DirectionalLightComponent& comp = m_app.GetComponentManager().GetComponent<DirectionalLightComponent>(entity);
    comp.Direction = _direction;
    comp.Color = _color;
    comp.Intensity = _intensity;
    return entity;
}

ecs::Entity LightSystem::CreatePointLight(const glm::vec3& _position, const glm::vec3& _color, float _intensity, const glm::vec3& _attenuation) const
{
    ecs::Entity entity = m_gameEntitySystem.CreateGameEntity(EntityType::PointLight);
    PointLightComponent& comp = m_app.GetComponentManager().GetComponent<PointLightComponent>(entity);
    comp.Position = _position;
    comp.Color = _color;
    comp.Intensity = _intensity;
    comp.Attenuation = _attenuation;
    return entity;
}

ecs::Entity LightSystem::CreateSpotLight(const glm::vec3& _position, const glm::vec3& _direction, const glm::vec3& _color, float _intensity, float _innerCutoff, float _outerCutoff) const
{
    ecs::Entity entity = m_gameEntitySystem.CreateGameEntity(EntityType::SpotLight);
    SpotLightComponent& comp = m_app.GetComponentManager().GetComponent<SpotLightComponent>(entity);
    comp.Position = _position;
    comp.Direction = _direction;
    comp.Color = _color;
    comp.Intensity = _intensity;
    comp.InnerCutoff = _innerCutoff;
    comp.OuterCutoff = _outerCutoff;
    return entity;
}

void LightSystem::FillLightsUBO(LightsUBO& _outLights) const
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();
    
    _outLights.DirectionalCount = 0;
    _outLights.PointCount = 0;
    _outLights.SpotCount = 0;
    
    for (auto entity : ecs::IterateEntitiesWithAll<DirectionalLightComponent>(entityManager, componentManager))
    {
        if (_outLights.DirectionalCount >= static_cast<int>(kMaxDirectionalLights)) break;
        const DirectionalLightComponent& comp = componentManager.GetComponent<DirectionalLightComponent>(entity);
        _outLights.DirectionalLights[_outLights.DirectionalCount].Direction = glm::vec4(comp.Direction, 0.0f);
        _outLights.DirectionalLights[_outLights.DirectionalCount].Color = glm::vec4(comp.Color, comp.Intensity);
        _outLights.DirectionalCount++;
    }

    for (auto entity : ecs::IterateEntitiesWithAll<PointLightComponent>(entityManager, componentManager))
    {
        if (_outLights.PointCount >= static_cast<int>(kMaxPointLights)) break;
        const PointLightComponent& comp = componentManager.GetComponent<PointLightComponent>(entity);
        _outLights.PointLights[_outLights.PointCount].Position = glm::vec4(comp.Position, 0.0f);
        _outLights.PointLights[_outLights.PointCount].Color = glm::vec4(comp.Color, comp.Intensity);
        _outLights.PointLights[_outLights.PointCount].Attenuation = glm::vec4(comp.Attenuation, 0.0f);
        _outLights.PointCount++;
    }

    for (auto entity : ecs::IterateEntitiesWithAll<SpotLightComponent>(entityManager, componentManager))
    {
        if (_outLights.SpotCount >= static_cast<int>(kMaxSpotLights)) break;
        const SpotLightComponent& comp = componentManager.GetComponent<SpotLightComponent>(entity);
        _outLights.SpotLights[_outLights.SpotCount].Position = glm::vec4(comp.Position, 0.0f);
        _outLights.SpotLights[_outLights.SpotCount].Direction = glm::vec4(comp.Direction, 0.0f);
        _outLights.SpotLights[_outLights.SpotCount].Color = glm::vec4(comp.Color, comp.Intensity);
        _outLights.SpotLights[_outLights.SpotCount].Params = glm::vec4(comp.InnerCutoff, comp.OuterCutoff, 0.0f, 0.0f);
        _outLights.SpotCount++;
    }
}

VESPERENGINE_NAMESPACE_END