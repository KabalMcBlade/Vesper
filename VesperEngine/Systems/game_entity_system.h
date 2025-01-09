// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\game_entity_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/camera_components.h"

#include "ECS/ECS/ecs.h"

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;

// this is the basic type, eventually the host application can add has many as it wants to the entity
enum EntityType : uint32
{
	Pure = 0,		// empty entity, logical purpose only
	Camera = 1,		// camera, has projection and special transform
	Object = 2,		// transform only
	Renderable = 3	// Object can be rendered
};

class VESPERENGINE_API GameEntitySystem final
{
public:
	GameEntitySystem(VesperApp& _app);
	~GameEntitySystem() = default;

	GameEntitySystem(const GameEntitySystem&) = delete;
	GameEntitySystem& operator=(const GameEntitySystem&) = delete;

public:
	ecs::Entity CreateGameEntity(EntityType _type) const;
	void DestroyGameEntity(const ecs::Entity _entity) const;
	void DestroyGameEntities() const;

private:
	VesperApp& m_app;
};

VESPERENGINE_NAMESPACE_END
