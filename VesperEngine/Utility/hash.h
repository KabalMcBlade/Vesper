// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\hash.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "ECS/ECS/hash.h"


VESPERENGINE_NAMESPACE_BEGIN

// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
void HashCombine(std::size_t& seed, const T& v, const Rest&... rest) 
{
	seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
	(HashCombine(seed, rest), ...);
};

template<typename T>
VESPERENGINE_INLINE uint32 ComponentToHash()
{
	const char* typeName = typeid(T).name();
	const uint32 hash = ecs::Hash(typeName, std::strlen(typeName));
	return hash;
}

VESPERENGINE_INLINE uint32 HashString(const std::string& _value)
{
	const uint32 hash = ecs::Hash(_value.c_str(), _value.length());
	return hash;
}

VESPERENGINE_NAMESPACE_END
