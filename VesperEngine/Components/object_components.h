#pragma once

#include "Core/core_defines.h"


#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/ext.hpp"


VESPERENGINE_NAMESPACE_BEGIN

struct TransformComponent
{
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale{ 1.f, 1.f, 1.f };
	glm::quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
};

// define an object which never change
// Although this is useful for graphic purpose, to use or not the staging buffer, is more logical to be on the "object" concept to be static
struct StaticComponent
{

};

VESPERENGINE_NAMESPACE_END
