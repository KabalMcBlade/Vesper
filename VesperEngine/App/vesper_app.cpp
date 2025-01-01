#include "vesper_app.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/camera_components.h"

#include "ECS/ECS/ecs.h"

VESPERENGINE_NAMESPACE_BEGIN

ecs::ComponentManager& VesperApp::GetComponentManager() 
{
	return m_componentManager; 
}

ecs::EntityManager& VesperApp::GetEntityManager()
{ 
	return m_entityManager; 
}

VesperApp::VesperApp(Config& _config)
	: m_componentManager(ecs::GetComponentManager())
	, m_entityManager(ecs::GetEntityManager())
	, m_config{ _config }
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
	m_entityManager.Create(m_config.MaxEntities);
	m_componentManager.Create(m_config.MaxEntities, m_config.MaxComponentsPerEntity);
}

void VesperApp::ShutdownECS()
{
	m_componentManager.Destroy();
	m_entityManager.Destroy();
}

void VesperApp::RegisterDefaultComponents()
{
	// CAMERA
	m_componentManager.RegisterComponent<CameraActive>();
	m_componentManager.RegisterComponent<CameraComponent>();
	m_componentManager.RegisterComponent<CameraTransformComponent>();

	// OBJECTS
	m_componentManager.RegisterComponent<TransformComponent>();
	m_componentManager.RegisterComponent<NoMaterialComponent>();
	m_componentManager.RegisterComponent<PhongMaterialComponent>();
	m_componentManager.RegisterComponent<PBRMaterialComponent>();
	m_componentManager.RegisterComponent<StaticComponent>();

	// BUFFERS
	m_componentManager.RegisterComponent<VertexBufferComponent>();
	m_componentManager.RegisterComponent<IndexBufferComponent>();
	m_componentManager.RegisterComponent<NotVertexBufferComponent>();
	m_componentManager.RegisterComponent<NotIndexBufferComponent>();

	// RENDER
	m_componentManager.RegisterComponent<RenderComponent>();
	m_componentManager.RegisterComponent<DynamicOffsetComponent>();
}

void VesperApp::UnregisterDefaultComponent()
{
	// CAMERA
	m_componentManager.UnregisterComponent<CameraActive>();
	m_componentManager.UnregisterComponent<CameraComponent>();
	m_componentManager.UnregisterComponent<CameraTransformComponent>();

	// OBJECTS
	m_componentManager.UnregisterComponent<TransformComponent>();
	m_componentManager.UnregisterComponent<NoMaterialComponent>();
	m_componentManager.UnregisterComponent<PhongMaterialComponent>();
	m_componentManager.UnregisterComponent<PBRMaterialComponent>();
	m_componentManager.UnregisterComponent<StaticComponent>();

	// BUFFERS
	m_componentManager.UnregisterComponent<VertexBufferComponent>();
	m_componentManager.UnregisterComponent<IndexBufferComponent>();
	m_componentManager.UnregisterComponent<NotVertexBufferComponent>();
	m_componentManager.UnregisterComponent<NotIndexBufferComponent>();

	// RENDER
	m_componentManager.UnregisterComponent<RenderComponent>();
	m_componentManager.UnregisterComponent<DynamicOffsetComponent>();
}

VESPERENGINE_NAMESPACE_END
