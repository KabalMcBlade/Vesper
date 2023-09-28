#pragma once

#include "Core/core_defines.h"


VESPERENGINE_NAMESPACE_BEGIN

// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
void HashCombine(std::size_t& seed, const T& v, const Rest&... rest) 
{
	seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
	(HashCombine(seed, rest), ...);
};

VESPERENGINE_NAMESPACE_END
