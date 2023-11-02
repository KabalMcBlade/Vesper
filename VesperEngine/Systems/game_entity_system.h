#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/camera_components.h"

#include "ECS/ecs.h"

VESPERENGINE_NAMESPACE_BEGIN

// this is the basic type, eventually the host application can add has many as it wants to the entity
enum EntityType : uint32
{
	Pure = 0,		// empty entity, logical purpose only
	Camera = 1,		// camera, has projection and special transform
	Object = 2		// transform only
};

class VESPERENGINE_DLL GameEntitySystem final
{
public:
	GameEntitySystem() = default;
	~GameEntitySystem() = default;

	GameEntitySystem(const GameEntitySystem&) = delete;
	GameEntitySystem& operator=(const GameEntitySystem&) = delete;

public:
	ecs::Entity CreateGameEntity(EntityType _type) const;
	void DestroyGameEntity(const ecs::Entity _entity) const;
	void DestroyGameEntities() const;
};

VESPERENGINE_NAMESPACE_END
