// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\logger.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Utility/logger.h"

#include <iostream>
#include <ctime>


VESPERENGINE_NAMESPACE_BEGIN

void Logger::Log(Level _level, const std::string& _message)
{
	std::cout 
		<< "[" << CurrentTime() << "]"
		<< "[" << LevelToString(_level) << "] "
		<< _message << std::endl;
}

void Logger::LogNewLine()
{
	std::cout
		<< "[" << CurrentTime() << "]"
		<< "[INFO] "
		<< std::endl;
}

std::string Logger::CurrentTime()
{
	std::time_t now = std::time(nullptr);
	std::tm localTime;
#if defined(_MSC_VER) // Use localtime_s on MSVC
	if (localtime_s(&localTime, &now) != 0) 
	{
		return "Unknown Time"; // Handle error
	}
#else // Use std::localtime on other platforms
	std::tm* localTimePtr = std::localtime(&now);
	if (!localTimePtr) {
		return "Unknown Time"; // Handle error
	}
	localTime = *localTimePtr; // Copy the result to avoid shared state issues
#endif
	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
	return buffer;
}

std::string Logger::LevelToString(Level _level)
{
	switch (_level)
	{
	case INFO: return "INFO";
	case WARNING: return "WARNING";
	case ERROR: return "ERROR";
	case DEBUG: return "DEBUG";
	default: return "UNKNOWN";
	}
}

VESPERENGINE_NAMESPACE_END
