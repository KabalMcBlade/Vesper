// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\App\file_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include <string>

VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API FileSystem
{
public:
	static bool IsAbsolutePath(const std::string& _filePath);
	static bool IsFilePath(const std::string& _filePath);
	static bool IsFileExists(const std::string& _filePath);
	static std::string GetDirectoryPath(const std::string& _filePath, bool _removeRightmostFolder = false);
	static std::string GetFileName(const std::string& _filePath, bool _withExtension = true);
	static bool HasExtension(const std::string& _filename);

private:
	FileSystem() = delete;
	~FileSystem() = delete;

	FileSystem(const FileSystem&) = delete;
	FileSystem& operator=(const FileSystem&) = delete;
};

VESPERENGINE_NAMESPACE_END
