#include "pch.h"
#include "App/window_handle.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/camera_components.h"

#include "ECS/ecs.h"

VESPERENGINE_NAMESPACE_BEGIN

WindowHandle::WindowHandle(
	int32 _width, int32 _height, std::string _name,
	uint16 _maxEntities, uint16 _maxComponentsPerEntity)
	: m_width{_width}
	, m_height{ _height }
	, m_name{ _name }
{
	// move this ECS stage to a better appropriate initializer stage rather than this window

	ecs::EntityManager::Create(_maxEntities);
	ecs::ComponentManager::Create(_maxEntities, _maxComponentsPerEntity);

	// basic components
	ecs::ComponentManager::RegisterComponent<CameraActive>();
	ecs::ComponentManager::RegisterComponent<CameraComponent>();
	ecs::ComponentManager::RegisterComponent<CameraTransformComponent>();
	ecs::ComponentManager::RegisterComponent<VertexBufferComponent>();
	ecs::ComponentManager::RegisterComponent<IndexBufferComponent>();
	ecs::ComponentManager::RegisterComponent<NotVertexBufferComponent>();
	ecs::ComponentManager::RegisterComponent<NotIndexBufferComponent>();
	ecs::ComponentManager::RegisterComponent<TransformComponent>();
	ecs::ComponentManager::RegisterComponent<MaterialComponent>();
	ecs::ComponentManager::RegisterComponent<StaticComponent>();
}

WindowHandle::~WindowHandle()
{
	ecs::ComponentManager::UnregisterComponent<StaticComponent>();
	ecs::ComponentManager::UnregisterComponent<MaterialComponent>();
	ecs::ComponentManager::UnregisterComponent<TransformComponent>();
	ecs::ComponentManager::UnregisterComponent<NotVertexBufferComponent>();
	ecs::ComponentManager::UnregisterComponent<NotIndexBufferComponent>();
	ecs::ComponentManager::UnregisterComponent<IndexBufferComponent>();
	ecs::ComponentManager::UnregisterComponent<VertexBufferComponent>();
	ecs::ComponentManager::UnregisterComponent<CameraTransformComponent>();
	ecs::ComponentManager::UnregisterComponent<CameraComponent>();
	ecs::ComponentManager::UnregisterComponent<CameraActive>();

	ecs::ComponentManager::Destroy();
	ecs::EntityManager::Destroy();
}

VESPERENGINE_NAMESPACE_END