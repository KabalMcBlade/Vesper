#include "pch.h"
#include "Backend/vertex.h"

#include "vulkan/vulkan.h"


VESPERENGINE_NAMESPACE_BEGIN

std::vector<VkVertexInputBindingDescription> Vertex::GetBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;	// vec4
	attributeDescriptions[0].offset = offsetof(Vertex, Position);

	attributeDescriptions[1].binding = 0;	// binding still 0, because the Position and Color are interleaving in the same binding
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;	// vec4
	attributeDescriptions[1].offset = offsetof(Vertex, Color);

	return attributeDescriptions;
}

VESPERENGINE_NAMESPACE_END
