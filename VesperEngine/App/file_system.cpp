// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\App\file_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "file_system.h"

#include <filesystem>

VESPERENGINE_NAMESPACE_BEGIN


bool FileSystem::IsAbsolutePath(const std::string& _filePath)
{
	return std::filesystem::path(_filePath).is_absolute();
}

bool FileSystem::IsFilePath(const std::string& _filePath)
{
	return !std::filesystem::path(_filePath).parent_path().empty();
}

bool FileSystem::IsFileExists(const std::string& _filePath)
{
	return std::filesystem::exists(_filePath) && std::filesystem::is_regular_file(_filePath);
}

std::string FileSystem::GetDirectoryPath(const std::string& _filePath, bool _removeRightmostFolder)
{
	std::filesystem::path path(_filePath);
	std::filesystem::path directory = path.parent_path();

	if (_removeRightmostFolder) 
	{
		directory = directory.parent_path();
	}

	std::string directoryStr = directory.string();
	if (!directoryStr.empty() && directoryStr.back() != '/' && directoryStr.back() != '\\') 
	{
		directoryStr += '/';
	}

	return directoryStr;
}

std::string FileSystem::GetFileName(const std::string& _filePath, bool _withExtension)
{
	std::filesystem::path path(_filePath);
	return _withExtension ? path.filename().string() : path.stem().string();
}

bool FileSystem::HasExtension(const std::string& _filename)
{
	size_t lastSlash = _filename.find_last_of("/\\");
	size_t lastDot = _filename.find_last_of('.');
	return (lastDot != std::string::npos) && (lastDot > lastSlash);
}


VESPERENGINE_NAMESPACE_END
