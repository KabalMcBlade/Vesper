// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\App\vesper_app.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"


namespace ecs {
	class ComponentManager;
	class EntityManager;
}

VESPERENGINE_NAMESPACE_BEGIN

struct Config;

class VESPERENGINE_API VesperApp
{
public:
	VesperApp(Config& _config);
	virtual ~VesperApp();

public:
	ecs::ComponentManager& GetComponentManager();
	ecs::EntityManager& GetEntityManager();

	VESPERENGINE_INLINE const Config& GetConfig() const { return m_config; }

private:
	void InitialieECS();
	void ShutdownECS();

	void RegisterDefaultComponents();
	void UnregisterDefaultComponent();

private:
	ecs::ComponentManager& m_componentManager;
	ecs::EntityManager& m_gameManager;
	Config& m_config;
};

VESPERENGINE_NAMESPACE_END
