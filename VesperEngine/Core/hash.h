// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Core\hash.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "ECS/ECS/hash.h"

#include "Core/core_defines.h"


VESPERENGINE_NAMESPACE_BEGIN

template<typename T>
VESPERENGINE_INLINE uint32 ComponentToHash()
{
	const char* typeName = typeid(T).name();
	const uint32 hash = ecs::Hash(typeName, std::strlen(typeName));
	return hash;
}

VESPERENGINE_NAMESPACE_END
