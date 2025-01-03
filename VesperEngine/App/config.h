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

	// Asset
	std::string ShadersFolderName = "Shaders/";
	std::string ModelsFolderName = "Models/";
	std::string TexturesFolderName = "Textures/";

	std::string ShadersPath = "Assets/" + ShadersFolderName;
	std::string ModelsPath = "Assets/" + ModelsFolderName;
	std::string TexturesPath = "Assets/" + TexturesFolderName;
};

VESPERENGINE_NAMESPACE_END
