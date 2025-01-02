#include "file_system.h"

#include <filesystem>

VESPERENGINE_NAMESPACE_BEGIN


bool FileSystem::IsAbsolutePath(const std::string& _filePath)
{
	return std::filesystem::path(_filePath).is_absolute();
}

std::string FileSystem::GetDirectoryPath(const std::string& _filePath)
{
	std::filesystem::path path(_filePath);
	std::string directory = path.parent_path().string();

	if (!directory.empty() && directory.back() != '/' && directory.back() != '\\')
	{
		directory += '/';
	}

	return directory;
}

bool FileSystem::HasExtension(const std::string& _filename)
{
	size_t lastSlash = _filename.find_last_of("/\\");
	size_t lastDot = _filename.find_last_of('.');
	return (lastDot != std::string::npos) && (lastDot > lastSlash);
}


VESPERENGINE_NAMESPACE_END
