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
