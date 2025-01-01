#pragma once

#include "Core/core_defines.h"

#include <string>
#include <filesystem>


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
	std::string ShadersPath = "Assets/Shaders/";
	std::string ModelsPath = "Assets/Models/";
	std::string TexturesPath = "Assets/Textures/";


	static VESPERENGINE_INLINE bool IsAbsolutePath(const std::string& _filePath)
	{
		return std::filesystem::path(_filePath).is_absolute();
	}

	static VESPERENGINE_INLINE std::string GetDirectoryPath(const std::string& _filePath)
	{
		std::filesystem::path path(_filePath);
		std::string directory = path.parent_path().string();

		if (!directory.empty() && directory.back() != '/' && directory.back() != '\\') 
		{
			directory += '/';
		}

		return directory;
	}
};

VESPERENGINE_NAMESPACE_END
