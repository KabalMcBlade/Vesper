#include "pch.h"
#include "App/window_handle.h"

#include "Components/core_components.h"

#include "ECS/ecs.h"

VESPERENGINE_NAMESPACE_BEGIN

WindowHandle::WindowHandle(
	int32 _width, int32 _height, std::string _name,
	uint16 _maxEntities, uint16 _maxComponentsPerEntity)
	: m_width{_width}
	, m_height{ _height }
	, m_name{ _name }
{
	ecs::EntityManager::Create(_maxEntities);
	ecs::ComponentManager::Create(_maxEntities, _maxComponentsPerEntity);

	// basic components
	ecs::ComponentManager::RegisterComponent<RenderComponent>();
	ecs::ComponentManager::RegisterComponent<TransformComponent>();
	ecs::ComponentManager::RegisterComponent<MaterialComponent>();
}

WindowHandle::~WindowHandle()
{
	ecs::ComponentManager::UnregisterComponent<MaterialComponent>();
	ecs::ComponentManager::UnregisterComponent<TransformComponent>();
	ecs::ComponentManager::UnregisterComponent<RenderComponent>();

	ecs::ComponentManager::Destroy();
	ecs::EntityManager::Destroy();
}

VESPERENGINE_NAMESPACE_END