#pragma once

#include "Core/core_defines.h"

#include <string>


VESPERENGINE_NAMESPACE_BEGIN

struct Config
{
	std::string WindowName = "";
	int32 WindowWidth = 800;
	int32 WindowHeight = 600;
};

VESPERENGINE_NAMESPACE_END
