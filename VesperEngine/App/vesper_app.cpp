#include "pch.h"
#include "vesper_app.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/camera_components.h"

#include "ECS/ecs.h"

VESPERENGINE_NAMESPACE_BEGIN

VesperApp::VesperApp(Config& _config)
	: m_config{ _config }
{
	InitialieECS();
	RegisterDefaultComponents();
}

VesperApp::~VesperApp()
{
	UnregisterDefaultComponent();
	ShutdownECS();
}

void VesperApp::InitialieECS()
{
	ecs::EntityManager::Create(m_config.MaxEntities);
	ecs::ComponentManager::Create(m_config.MaxEntities, m_config.MaxComponentsPerEntity);
}

void VesperApp::ShutdownECS()
{
	ecs::ComponentManager::Destroy();
	ecs::EntityManager::Destroy();
}

void VesperApp::RegisterDefaultComponents()
{
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

void VesperApp::UnregisterDefaultComponent()
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
}

VESPERENGINE_NAMESPACE_END
