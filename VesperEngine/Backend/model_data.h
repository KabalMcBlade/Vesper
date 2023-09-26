#pragma once

#include "Core/core_defines.h"

#include "vulkan/vulkan.h"

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Color;

	static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
};

struct ModelData
{
	std::vector<Vertex> Vertices{};
	std::vector<uint32> Indices{};
	bool IsStatic{false};
};

VESPERENGINE_NAMESPACE_END
