// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Components\light_components.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"


VESPERENGINE_NAMESPACE_BEGIN


/*
    struct DirectionalLightData { vec4 Direction; vec4 Color; };
    struct PointLightData { vec4 Position; vec4 Color; vec4 Attenuation; };
    struct SpotLightData { vec4 Position; vec4 Direction; vec4 Color; vec4 Params; };
*/

struct DirectionalLightComponent
{
    glm::vec3 Direction{ 0.0f, -1.0f, 0.0f };
    glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
    float Intensity{ 1.0f };
};

struct PointLightComponent
{
    glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
    float Intensity{ 1.0f };
    glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
    glm::vec3 Attenuation{ 1.0f, 0.0f, 0.0f }; // constant, linear, quadratic
};

struct SpotLightComponent
{
    glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
    float Intensity{ 1.0f };
    glm::vec3 Direction{ 0.0f, -1.0f, 0.0f };
    float InnerCutoff{ 0.8f };
    glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
    float OuterCutoff{ 0.9f };
};

VESPERENGINE_NAMESPACE_END
