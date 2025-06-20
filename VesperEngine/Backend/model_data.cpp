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
    attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT , offsetof(Vertex, UV) });
    attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphPos[0]) });
    attributeDescriptions.push_back({ 5, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphNorm[0]) });
    attributeDescriptions.push_back({ 6, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphPos[1]) });
    attributeDescriptions.push_back({ 7, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphNorm[1]) });
    attributeDescriptions.push_back({ 8, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphPos[2]) });
    attributeDescriptions.push_back({ 9, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphNorm[2]) });
    attributeDescriptions.push_back({ 10, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphPos[3]) });
    attributeDescriptions.push_back({ 11, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphNorm[3]) });
    attributeDescriptions.push_back({ 12, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphPos[4]) });
    attributeDescriptions.push_back({ 13, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphNorm[4]) });
    attributeDescriptions.push_back({ 14, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphPos[5]) });
    attributeDescriptions.push_back({ 15, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphNorm[5]) });
    attributeDescriptions.push_back({ 16, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphPos[6]) });
    attributeDescriptions.push_back({ 17, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphNorm[6]) });
    attributeDescriptions.push_back({ 18, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphPos[7]) });
    attributeDescriptions.push_back({ 19, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, MorphNorm[7]) });


	return attributeDescriptions;
}

VESPERENGINE_NAMESPACE_END
