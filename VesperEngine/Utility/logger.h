// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\logger.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include <string>
#include <sstream>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API Logger
{
public:
	enum Level
	{
		INFO,
		WARNING,
		ERROR,
		DEBUG
	};

	static void Log(Level _level, const std::string& _message);
	static void LogNewLine();

	template <typename... Args>
	static void LogVariadic(Level _level, Args&&... args)
	{
		std::ostringstream oss;
		(oss << ... << std::forward<Args>(args));
		Log(_level, oss.str());
	}

private:
	static std::string CurrentTime();
	static std::string LevelToString(Level _level);
};

VESPERENGINE_NAMESPACE_END


#ifdef _DEBUG
	#define LOG(Level, ...) vesper::Logger::LogVariadic(Level, __VA_ARGS__)
	#define LOG_NL() vesper::Logger::LogNewLine()
#else
	#define LOG(Level, ...) 
	#define LOG_NL()
#endif