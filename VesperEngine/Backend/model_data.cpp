// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\model_data.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Backend/model_data.h"

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
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, Position) });
    attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, Color) });
    attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, Normal) });
    attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT , offsetof(Vertex, UV1) });
    attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32_SFLOAT , offsetof(Vertex, UV2) });
    attributeDescriptions.push_back({ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT , offsetof(Vertex, Tangent) });
    size_t morphPosOffset = offsetof(Vertex, MorphPos);
    size_t morphNormOffset = offsetof(Vertex, MorphNorm);
    for (uint32 i = 0; i < kMaxMorphTargets; ++i)
    {
        attributeDescriptions.push_back({ static_cast<uint32>(6 + i * 2), 0, VK_FORMAT_R32G32B32_SFLOAT , static_cast<uint32>(morphPosOffset + sizeof(glm::vec3) * i) });
        attributeDescriptions.push_back({ static_cast<uint32>(7 + i * 2), 0, VK_FORMAT_R32G32B32_SFLOAT , static_cast<uint32>(morphNormOffset + sizeof(glm::vec3) * i) });
    }


	return attributeDescriptions;
}

VESPERENGINE_NAMESPACE_END
