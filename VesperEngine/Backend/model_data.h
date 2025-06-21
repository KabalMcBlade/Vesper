// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\model_data.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"

#include "Components/graphics_components.h"

#include "Systems/uniform_buffer.h"

#include "vma/vk_mem_alloc.h"

#include <vector>
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN


struct Vertex
{
	glm::vec3 Position{};
	glm::vec3 Color{};
	glm::vec3 Normal{};
	glm::vec2 UV1{};
	glm::vec2 UV2{};
	glm::vec3 MorphPos[kMaxMorphTargets]{};
	glm::vec3 MorphNorm[kMaxMorphTargets]{};

	static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

	bool operator==(const Vertex& _other) const
	{
		bool equal = Position == _other.Position && Color == _other.Color && Normal == _other.Normal 
			&& UV1 == _other.UV1 && UV2 == _other.UV2;
		for (int i = 0; i < static_cast<int>(kMaxMorphTargets) && equal; ++i)
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
	std::vector<int32> UVIndices;
	BufferComponent UniformBuffer;
	int32 Index{ -1 };
	bool IsTransparent{ false };
	MaterialType Type;
};

struct MorphKeyframe
{
	float Time{ 0.0f };
	glm::vec4 Weights[2]{ glm::vec4(0.0f), glm::vec4(0.0f) };
};

struct MorphAnimation
{
	std::string Name{};
	std::vector<MorphKeyframe> Keyframes{};
};

struct ModelData
{
	std::vector<Vertex> Vertices{};
	std::vector<uint32> Indices{};
	std::shared_ptr<MaterialData> Material;
	bool IsStatic{ false };
	glm::vec4 MorphWeights[2]{ glm::vec4(0.0f), glm::vec4(0.0f) };
	uint32 MorphTargetCount{ 0 };
	std::vector<MorphAnimation> Animations{};
};

VESPERENGINE_NAMESPACE_END
