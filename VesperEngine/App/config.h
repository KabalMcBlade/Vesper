#pragma once

#include "Core/core_defines.h"

#include <string>


VESPERENGINE_NAMESPACE_BEGIN

struct Config
{
	// Window
	std::string WindowName = "";
	int32 WindowWidth = 800;
	int32 WindowHeight = 600;

	// ECS
	uint16 MaxEntities = 128;
	uint16 MaxComponentsPerEntity = 32;
};

VESPERENGINE_NAMESPACE_END
