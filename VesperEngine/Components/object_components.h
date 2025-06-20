// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Components\object_components.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"

#include "ECS/ECS/entity.h"


VESPERENGINE_NAMESPACE_BEGIN

struct TransformComponent
{
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale{ 1.f, 1.f, 1.f };
	glm::quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

	ecs::Entity Parent = ecs::UnknowEntity;
	std::vector<ecs::Entity> Children;
};

// define an object which never change
// Although this is useful for graphic purpose, to use or not the staging buffer, is more logical to be on the "object" concept to be static
struct StaticComponent
{

};

// define the struct which each instance has at least to have, if needs to be updated
struct UpdateComponent
{
	glm::mat4 ModelMatrix{ 1 };
};

// define the struct which each instance has at least to have, if needs to be visible
struct VisibilityComponent
{
};

struct MorphWeightsComponent
{
	glm::vec4 Weights[2]{ glm::vec4(0.0f), glm::vec4(0.0f) };
	uint32 Count{ 0 };
};

VESPERENGINE_NAMESPACE_END
