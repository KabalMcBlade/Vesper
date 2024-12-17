#pragma once

#include "Core/core_defines.h"
#include "App/config.h"


namespace ecs {
	class ComponentManager;
	class EntityManager;
}

VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL VesperApp
{
public:
	VesperApp(Config& _config);
	virtual ~VesperApp();

public:
	ecs::ComponentManager& GetComponentManager();
	ecs::EntityManager& GetEntityManager();

private:
	void InitialieECS();
	void ShutdownECS();

	void RegisterDefaultComponents();
	void UnregisterDefaultComponent();

private:
	ecs::ComponentManager& m_componentManager;
	ecs::EntityManager& m_entityManager;
	Config& m_config;
};

VESPERENGINE_NAMESPACE_END
