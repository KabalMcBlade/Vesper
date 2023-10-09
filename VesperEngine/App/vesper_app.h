#pragma once

#include "Core/core_defines.h"
#include "App/config.h"


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL VesperApp
{
public:
	VesperApp(Config& _config);
	virtual ~VesperApp();

private:
	void InitialieECS();
	void ShutdownECS();

	void RegisterDefaultComponents();
	void UnregisterDefaultComponent();

private:
	Config& m_config;
};

VESPERENGINE_NAMESPACE_END
