// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\model_data.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"

#include "Components/graphics_components.h"

#include "vma/vk_mem_alloc.h"

#include <vector>
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

struct Vertex
{
	glm::vec3 Position{};
	glm::vec3 Color{};
	glm::vec3 Normal{};
	glm::vec2 UV{};
	glm::vec3 MorphPos[4]{};
	glm::vec3 MorphNorm[4]{};

	static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

	bool operator==(const Vertex& _other) const
	{
		bool equal = Position == _other.Position && Color == _other.Color && Normal == _other.Normal && UV == _other.UV;
		for (int i = 0;i < 4 && equal;++i)
		{
			equal = equal && MorphPos[i] == _other.MorphPos[i] && MorphNorm[i] == _other.MorphNorm[i];
		}
		return equal;
	}
};

struct TextureData
{
	VmaAllocation AllocationMemory{ VK_NULL_HANDLE };	// Memory allocation handled by VMA
	VkImage Image{ VK_NULL_HANDLE };					// The Vulkan image
	VkImageView ImageView{ VK_NULL_HANDLE };			// The image view for shader access
	VkSampler Sampler{ VK_NULL_HANDLE };				// The sampler for texture filtering
	int32 Index{ -1 };
};

enum class MaterialType : uint32
{
	Phong,
	PBR
};

struct MaterialData
{
#ifdef _DEBUG
	std::string Name{ "" };
#endif
	std::vector<std::shared_ptr<TextureData>> Textures;
	BufferComponent UniformBuffer;
	int32 Index{ -1 };
	bool IsTransparent{ false };
	MaterialType Type;
};

struct ModelData
{
	std::vector<Vertex> Vertices{};
	std::vector<uint32> Indices{};
	std::shared_ptr<MaterialData> Material;
	bool IsStatic{ false };
	glm::vec4 MorphWeights{ 0.0f };
	uint32 MorphTargetCount{ 0 };
};

VESPERENGINE_NAMESPACE_END
