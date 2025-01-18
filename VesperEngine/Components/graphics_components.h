// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Components\graphics_components.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "vma/vk_mem_alloc.h"

#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

// Pure struct, means is not registered and MUST not be registered.
// Used as base structures for buffers
// Is also useful to move around or use as buffer containers, as long as is not directly used as component for entity (ECS)
// Could be added to the list, but this is pure for temporary usage
struct BufferComponent
{
	VmaAllocation AllocationMemory{ VK_NULL_HANDLE };	// value storing the allocation to map/unmap	
	VkBuffer Buffer{ VK_NULL_HANDLE };					// value used to bind and draw
	VkDeviceSize Size{ 0 };								// the actual size of the current buffer
	VkDeviceSize AlignedSize{ 0 };						// Actual aligned size for uniform buffer, if is not required alignment, is equal to Size
	uint32 Count{ 0 };									// instance count
	void* MappedMemory{ nullptr };						// Pointer for mapped memory (if needed)
};

struct VertexBufferComponent : public BufferComponent
{
};

// Special struct to set if we want to mark a render-able object having NOT a VB.
// To be fair, does not make sense, but to be aligned with the IB, I added as well.
// 
// IMPORTANT!!!
// This struct is a "toggle", NEED to be add IF AND ONLY IF VertexBufferComponent IS NOT PRESENT
struct NotVertexBufferComponent
{

};

struct IndexBufferComponent : public BufferComponent
{
};

// 
// IMPORTANT!!!
// This struct is a "toggle", NEED to be add IF AND ONLY IF IndexBufferComponent IS NOT PRESENT
struct NotIndexBufferComponent
{

};


// Pure struct, means is not registered and MUST not be registered.
// Used as base structures for Materials
struct MaterialComponent
{
	using FieldType = int32;
	int32 Index{ -1 };

	// store the descriptor set bound to the resource per presentation frame (1,2 or 3)
	std::vector<VkDescriptorSet> BoundDescriptorSet;
};

// Special, to render without material
struct NoMaterialComponent
{
};

// used with PhongRenderSystem
struct PhongMaterialComponent : public MaterialComponent
{
	VkDescriptorImageInfo DiffuseImageInfo{};
	VkDescriptorImageInfo SpecularImageInfo{};
	VkDescriptorImageInfo AmbientImageInfo{};
	VkDescriptorImageInfo NormalImageInfo{};
	VkDescriptorBufferInfo UniformBufferInfo{};	// colors/values
};

// used with PBRRenderSystem
struct PBRMaterialComponent : public MaterialComponent
{
	VkDescriptorImageInfo RoughnessImageInfo{};
	VkDescriptorImageInfo MetallicImageInfo{};
	VkDescriptorImageInfo SheenImageInfo{};
	VkDescriptorImageInfo EmissiveImageInfo{};
	VkDescriptorImageInfo NormalImageInfo{};
	VkDescriptorBufferInfo UniformBufferInfo{};	// colors/values
};

struct DynamicOffsetComponent
{
	uint32 DynamicOffsetIndex{ 0 };
	uint32 DynamicOffset{ 0 };
};

VESPERENGINE_NAMESPACE_END
