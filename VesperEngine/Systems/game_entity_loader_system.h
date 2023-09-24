#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/vertex.h"
#include "Backend/pipeline.h"
#include "App/window_handle.h"

#include "ECS/ecs.h"

#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL GameEntityLoaderSystem
{
public:
	GameEntityLoaderSystem(Device& _device);
	~GameEntityLoaderSystem();

	GameEntityLoaderSystem(const GameEntityLoaderSystem&) = delete;
	GameEntityLoaderSystem& operator=(const GameEntityLoaderSystem&) = delete;

public:
	ecs::Entity CreateGameEntity();
	void DestroyGameEntity(const ecs::Entity _entity);
	void DestroyGameEntities();

	void LoadGameEntity(ecs::Entity _entity, const std::vector<Vertex> _vertices);
	void UnloadGameEntity(ecs::Entity _entity);

	// TEST: all entities use the same vertex description using RenderComponent, TransformComponent and MaterialComponent
	void LoadGameEntities(const std::vector<Vertex> _vertices);
	void UnloadGameEntities();

private:
	Device& m_device;
};

VESPERENGINE_NAMESPACE_END
