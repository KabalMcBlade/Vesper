// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\light_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Components/light_components.h"

#include "ECS/ECS/entity.h"

#include <vector>

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class GameEntitySystem;

struct LightsUBO;

class VESPERENGINE_API LightSystem final
{
public:
    LightSystem(VesperApp& _app, GameEntitySystem& _gameEntitySystem);
    ~LightSystem() = default;

    LightSystem(const LightSystem&) = delete;
    LightSystem& operator=(const LightSystem&) = delete;

public:
    ecs::Entity CreateDirectionalLight(const glm::vec3& _direction, const glm::vec3& _color, float _intensity) const;
    ecs::Entity CreatePointLight(const glm::vec3& _position, const glm::vec3& _color, float _intensity, const glm::vec3& _attenuation) const;
    ecs::Entity CreateSpotLight(const glm::vec3& _position, const glm::vec3& _direction, const glm::vec3& _color, float _intensity, float _innerCutoff, float _outerCutoff) const;

    void FillLightsUBO(LightsUBO& _outLights) const;

private:
    VesperApp& m_app;
    GameEntitySystem& m_gameEntitySystem;
};

VESPERENGINE_NAMESPACE_END