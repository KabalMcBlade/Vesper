// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\texture_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "texture_system.h"

#include "Backend/offscreen_renderer.h"

#include "Systems/brdf_lut_generation_system.h"

#include "App/file_system.h"

#include "Utility/hash.h"
#include "Utility/logger.h"

#include "../stb/stb_image.h"


VESPERENGINE_NAMESPACE_BEGIN

const TextureSystem::DefaultTextureType TextureSystem::NormalTexture =
{
	"_NormalTexture_",
	{ 128, 128, 255, 255 },
	VK_FORMAT_R8G8B8A8_UNORM,
	1, 1
};

const TextureSystem::DefaultTextureType TextureSystem::DiffuseTexture =
{
	"_DiffuseTexture_",
	{ 255, 255, 255, 255 },
	VK_FORMAT_R8G8B8A8_SRGB,
	1, 1
};

const TextureSystem::DefaultTextureType TextureSystem::SpecularTexture =
{
	"_SpecularTexture_",
	 { 0, 0, 0, 255 },
	VK_FORMAT_R8G8B8A8_UNORM,
	1, 1
};

const TextureSystem::DefaultTextureType TextureSystem::AmbientTexture =
{
	"_AmbientTexture_",
	{ 128, 128, 128, 255 },
	VK_FORMAT_R8G8B8A8_SRGB,
	1, 1
};

const TextureSystem::DefaultTextureType TextureSystem::RoughnessTexture =
{
	"_RoughnessTexture_",
	{ 255, 255, 255, 255 },
	VK_FORMAT_R8G8B8A8_UNORM,
	1, 1
};

const TextureSystem::DefaultTextureType TextureSystem::MetallicTexture =
{
	"_MetallicTexture_",
	{ 0, 0, 0, 255 },
	VK_FORMAT_R8G8B8A8_UNORM,
	1, 1
};

const TextureSystem::DefaultTextureType TextureSystem::SheenTexture =
{
	"_SheenTexture_",
	{ 128, 128, 128, 255 },
	VK_FORMAT_R8G8B8A8_UNORM,
	1, 1
};

const TextureSystem::DefaultTextureType TextureSystem::EmissiveTexture =
{
	"_EmissiveTexture_",
	{ 0, 0, 0, 255 },
	VK_FORMAT_R8G8B8A8_SRGB,
	1, 1
};


TextureSystem::TextureSystem(Device& _device)
	: m_device(_device)
{
	m_buffer = std::make_unique<Buffer>(m_device);
}

std::shared_ptr<TextureData> TextureSystem::LoadTexture(const std::string& _path, VkFormat _overrideFormat)
{
	const uint32 hash = HashString(_path.c_str());
		
	auto it = m_textureLookup.find(hash);
	if (it != m_textureLookup.end())
	{
		const int32 index = it->second;
		return m_textures[index];
	}

	auto texture = std::make_shared<TextureData>();

	// Step 1: Load texture data
	int32 width, height, channels;
	uint8* data = LoadTextureData(_path, width, height, channels, STBI_rgb_alpha);
	if (!data)
	{
		throw std::runtime_error("Failed to load texture: " + _path);
	}

	// Step 2: Determine Vulkan format
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // Default to SRGB 4 channels
	if (_overrideFormat == VK_FORMAT_UNDEFINED)
	{
		if (channels == 1)
		{
			format = VK_FORMAT_R8_UNORM; // Single channel (gray-scale)
		}
		else if (channels == 3)
		{
			format = VK_FORMAT_R8G8B8A8_SRGB; // Convert RGB to RGBA
		}
	}
	else
	{
		format = _overrideFormat;
	}

	// Step 3: Create Vulkan texture image
	CreateTextureImage(data, width, height, format, texture->Image, texture->AllocationMemory);

	// Step 4: Create Vulkan image view
	texture->ImageView = CreateImageView(texture->Image, format);

	// Step 5: Create Vulkan texture sampler
	texture->Sampler = CreateTextureSampler();

	// Free the loaded texture data
	FreeTextureData(data);

	m_textures.push_back(texture);

	const int32 index = static_cast<int32>(m_textures.size() - 1);
	m_textureLookup[hash] = index;

	texture->Index = index;

	return texture;
}

std::shared_ptr<TextureData> TextureSystem::LoadTexture(const std::string& _name, VkFormat _format, const uint8* _data, int32 _width, int32 _height)
{
	const uint32 hash = HashString(_name.c_str());

	auto it = m_textureLookup.find(hash);
	if (it != m_textureLookup.end())
	{
		const int32 index = it->second;
		return m_textures[index];
	}

	auto texture = std::make_shared<TextureData>();

	CreateTextureImage(_data, _width, _height, _format, texture->Image, texture->AllocationMemory);
	texture->ImageView = CreateImageView(texture->Image, _format);
	texture->Sampler = CreateTextureSampler();

	m_textures.push_back(texture);

	const int32 index = static_cast<int32>(m_textures.size() - 1);
	m_textureLookup[hash] = index;

	texture->Index = index;

	return texture;
}

std::shared_ptr<TextureData> TextureSystem::LoadTexture(const DefaultTextureType& _defaultTexture)
{
	return LoadTexture(_defaultTexture.Name, _defaultTexture.Format, _defaultTexture.Buffer.data(), _defaultTexture.Width, _defaultTexture.Height);
}

std::shared_ptr<TextureData> TextureSystem::GenerateOrLoadBRDFLutTexture(VesperApp& _app, const std::string& _saveLoadPath, VkExtent2D _extent)
{
	const uint32 hash = HashString(_saveLoadPath.c_str());

	auto it = m_textureLookup.find(hash);
	if (it != m_textureLookup.end())
	{
		const int32 index = it->second;
		return m_textures[index];
	}

	std::unique_ptr<OffscreenRenderer> offscreenRenderer = std::make_unique<OffscreenRenderer>(m_device, _extent, VK_FORMAT_R8G8B8A8_UNORM);
	std::unique_ptr<BRDFLUTGenerationSystem> m_brdfLutGenerationSystem = std::make_unique<BRDFLUTGenerationSystem>(_app, m_device, offscreenRenderer->GetOffscreenSwapChainRenderPass());

	auto commandBuffer = offscreenRenderer->BeginFrame();
	offscreenRenderer->BeginOffscreenSwapChainRenderPass(commandBuffer);

	m_brdfLutGenerationSystem->Generate(commandBuffer, _extent.width, _extent.height);

	offscreenRenderer->EndOffscreenSwapChainRenderPass(commandBuffer);

	BufferComponent stagingBuffer = offscreenRenderer->PrepareImageCopy(commandBuffer);

	offscreenRenderer->EndFrame();

	offscreenRenderer->FlushBufferToFile(_saveLoadPath, stagingBuffer);

	return LoadTexture(_saveLoadPath, VK_FORMAT_R8G8B8A8_UNORM);
}

uint8* TextureSystem::LoadTextureData(const std::string& _path, int32& _width, int32& _height, int32& _channels, int32 _desired_channels)
{
	uint8* data = stbi_load(_path.c_str(), &_width, &_height, &_channels, _desired_channels);
	if (!data)
	{
		throw std::runtime_error("Failed to load texture file: " + _path);
	}
	return data;
}

void TextureSystem::FreeTextureData(uint8* _data)
{
	stbi_image_free(_data);
}

void TextureSystem::CreateTextureImage(const uint8* _data, int32 _width, int32 _height, VkFormat _format, VkImage& _image, VmaAllocation& _allocation)
{
	VkDeviceSize imageSize = _width * _height * 4; // Assuming 4 bytes per pixel (RGBA)

	BufferComponent stagingBuffer = m_buffer->Create<BufferComponent>(
		imageSize,
		1,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	m_buffer->Map(stagingBuffer);
	m_buffer->WriteToBuffer(stagingBuffer.MappedMemory, (void*)_data, static_cast<size_t>(imageSize));
	m_buffer->Unmap(stagingBuffer);


	// Create the Vulkan image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32>(_width);
	imageInfo.extent.height = static_cast<uint32>(_height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = _format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// VMA_MEMORY_USAGE_GPU_ONLY - Check internal usage for VmaAllocationCreateInfo
	m_device.CreateImageWithInfo(imageInfo, _image, _allocation);

	// Copy data from staging buffer to Vulkan image
	VkCommandBuffer commandBuffer = m_device.BeginSingleTimeCommands();

	m_device.TransitionImageLayout(commandBuffer, _image, _format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	m_device.CopyBufferToImage(commandBuffer, stagingBuffer.Buffer, _image, _width, _height, 1);
	m_device.TransitionImageLayout(commandBuffer, _image, _format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_device.EndSingleTimeCommands(commandBuffer);

	// Cleanup staging buffer
	m_buffer->Destroy(stagingBuffer);
}

VkImageView TextureSystem::CreateImageView(VkImage _image, VkFormat _format)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = _image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = _format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(m_device.GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan image view");
	}

	return imageView;
}

VkSampler TextureSystem::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16; // Adjust as per GPU capabilities
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;	// VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;

	VkSampler sampler;
	if (vkCreateSampler(m_device.GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}

	return sampler;
}

void TextureSystem::Cleanup()
{
	for (const auto& texture : m_textures)
	{
		vkDestroyImageView(m_device.GetDevice(), texture->ImageView, nullptr);
		vkDestroySampler(m_device.GetDevice(), texture->Sampler, nullptr);
		vmaDestroyImage(m_device.GetAllocator(), texture->Image, texture->AllocationMemory);
	}

	m_textures.clear();
	m_textureLookup.clear();
}

VESPERENGINE_NAMESPACE_END
