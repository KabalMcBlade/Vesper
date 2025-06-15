// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\texture_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once


#include "Core/core_defines.h"

#include "Backend/buffer.h"
#include "Backend/model_data.h"

#include "vulkan/vulkan.h"

#include <unordered_map>
#include <memory>
#include <array>
#include <string>
#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;

class VESPERENGINE_API TextureSystem final
{
private:
	struct DefaultTextureType
	{
		std::string Name;
		std::array<uint8, 4u> Buffer;
		VkFormat Format;
		int32 Width;
		int32 Height;
	};

public:
	// Common default textures
	static const DefaultTextureType NormalTexture;

	// Phong default textures
	static const DefaultTextureType DiffuseTexture;
	static const DefaultTextureType SpecularTexture;
    static const DefaultTextureType AmbientTexture;
    static const DefaultTextureType AlphaTexture;

	// PBR default textures
	static const DefaultTextureType RoughnessTexture;
	static const DefaultTextureType MetallicTexture;
	static const DefaultTextureType SheenTexture;
	static const DefaultTextureType EmissiveTexture;

public:
	static int32 GetBytesPerPixel(VkFormat _format);

	template <typename T>
	static void ValidateFormatCompatibility(VkFormat format)
	{
		// Handle uint8_t formats
		if constexpr (std::is_same<T, uint8_t>::value)
		{
			if (format != VK_FORMAT_R8_UNORM && format != VK_FORMAT_R8_SNORM &&
				format != VK_FORMAT_R8_UINT && format != VK_FORMAT_R8_SRGB &&
				format != VK_FORMAT_R8G8_UNORM && format != VK_FORMAT_R8G8_SNORM &&
				format != VK_FORMAT_R8G8_UINT && format != VK_FORMAT_R8G8_SRGB &&
				format != VK_FORMAT_R8G8B8_UNORM && format != VK_FORMAT_R8G8B8_SNORM &&
				format != VK_FORMAT_R8G8B8_UINT && format != VK_FORMAT_R8G8B8_SRGB &&
				format != VK_FORMAT_R8G8B8A8_UNORM && format != VK_FORMAT_R8G8B8A8_SNORM &&
				format != VK_FORMAT_R8G8B8A8_UINT && format != VK_FORMAT_R8G8B8A8_SRGB)
			{
				throw std::invalid_argument("Format is incompatible with uint8_t data type.");
			}
		}
		// Handle int8_t formats
		else if constexpr (std::is_same<T, int8_t>::value)
		{
			if (format != VK_FORMAT_R8_SINT && format != VK_FORMAT_R8G8_SINT &&
				format != VK_FORMAT_R8G8B8_SINT && format != VK_FORMAT_R8G8B8A8_SINT)
			{
				throw std::invalid_argument("Format is incompatible with int8_t data type.");
			}
		}
		// Handle float formats
		else if constexpr (std::is_same<T, float>::value)
		{
			if (format != VK_FORMAT_R16_SFLOAT && format != VK_FORMAT_R16G16_SFLOAT &&
				format != VK_FORMAT_R16G16B16_SFLOAT && format != VK_FORMAT_R16G16B16A16_SFLOAT &&
				format != VK_FORMAT_R32_SFLOAT && format != VK_FORMAT_R32G32_SFLOAT &&
				format != VK_FORMAT_R32G32B32_SFLOAT && format != VK_FORMAT_R32G32B32A32_SFLOAT)
			{
				throw std::invalid_argument("Format is incompatible with float data type.");
			}
		}
		else if constexpr (std::is_same<T, uint16_t>::value)
		{
			if (format != VK_FORMAT_R16_UNORM && format != VK_FORMAT_R16_SNORM && format != VK_FORMAT_R16_UINT && format != VK_FORMAT_R16_SINT && format != VK_FORMAT_R16_SFLOAT &&
				format != VK_FORMAT_R16G16_UNORM && format != VK_FORMAT_R16G16_SNORM && format != VK_FORMAT_R16G16_UINT && format != VK_FORMAT_R16G16_SINT && format != VK_FORMAT_R16G16_SFLOAT &&
				format != VK_FORMAT_R16G16B16_UNORM && format != VK_FORMAT_R16G16B16_SNORM && format != VK_FORMAT_R16G16B16_UINT && format != VK_FORMAT_R16G16B16_SINT && format != VK_FORMAT_R16G16B16_SFLOAT &&
				format != VK_FORMAT_R16G16B16A16_UNORM && format != VK_FORMAT_R16G16B16A16_SNORM && format != VK_FORMAT_R16G16B16A16_UINT && format != VK_FORMAT_R16G16B16A16_SINT && format != VK_FORMAT_R16G16B16A16_SFLOAT)
			{
				throw std::invalid_argument("Format is incompatible with uint16_t data type.");
			}
		}
		// Handle int32_t formats
		else if constexpr (std::is_same<T, int32_t>::value)
		{
			if (format != VK_FORMAT_R32_SINT && format != VK_FORMAT_R32G32_SINT &&
				format != VK_FORMAT_R32G32B32_SINT && format != VK_FORMAT_R32G32B32A32_SINT)
			{
				throw std::invalid_argument("Format is incompatible with int32_t data type.");
			}
		}
		else
		{
			throw std::invalid_argument("Unsupported data type.");
		}
	}


public:
	TextureSystem(VesperApp& _app, Device& _device);
	~TextureSystem() = default;

	TextureSystem(const TextureSystem&) = delete;
	TextureSystem& operator=(const TextureSystem&) = delete;

	// if _overrideFormat is different from VK_FORMAT_UNDEFINED, it uses whatever has been passed, 
	// otherwise use channels definition to figure out what format to use
	std::shared_ptr<TextureData> LoadTexture(const std::string& _path, VkFormat _overrideFormat = VK_FORMAT_UNDEFINED);
	std::shared_ptr<TextureData> LoadTexture(const std::string& _name, VkFormat _format, const uint8* _data, int32 _width, int32 _height);
	std::shared_ptr<TextureData> LoadTexture(const DefaultTextureType& _defaultTexture);
	std::shared_ptr<TextureData> LoadCubemap(const std::array<std::string, 6>& _paths, VkFormat _overrideFormat = VK_FORMAT_UNDEFINED);
	std::shared_ptr<TextureData> LoadCubemap(const std::string& _hdrPath, VkFormat _overrideFormat = VK_FORMAT_UNDEFINED);
	void Cleanup();

	std::shared_ptr<TextureData> GenerateOrLoadBRDFLutTexture(const std::string& _saveLoadPath, VkExtent2D _extent);

	VESPERENGINE_INLINE std::shared_ptr<TextureData> GetTexture(uint32 _index) const
	{
		assert(_index >= 0 && _index < m_textures.size() && "Texture index is out of bound!");
		return m_textures[_index];
	}

	VESPERENGINE_INLINE const std::vector<std::shared_ptr<TextureData>>& GetTextures() const
	{
		return m_textures;
	}

	VESPERENGINE_INLINE const int32 GetTextureIndex(const std::shared_ptr<TextureData>& _texture) const
	{
		auto it = std::find(m_textures.begin(), m_textures.end(), _texture);
		if (it != m_textures.end())
		{
			return static_cast<int32>(std::distance(m_textures.begin(), it));
		}

		return -1;
	}

private:
	uint8* LoadTextureData(const std::string& _path, int32& _width, int32& _height, int32& _channels, int32 _desired_channels);
	void FreeTextureData(uint8* _data);

	template <typename T>
	void CreateTextureImage(const T* _data, int32 _width, int32 _height, VkFormat _format, VkImage& _image, VmaAllocation& _allocation,
							uint32 _layerCount = 1, uint32 _mipLevels = 1, VkImageCreateFlags _flags = 0)
	{
		static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, float>::value || std::is_same<T, uint16_t>::value, "Unsupported data type. Only uint8, uint16 and float are allowed.");

#ifdef _DEBUG
		ValidateFormatCompatibility<T>(_format);
#endif

		const int32 bpp = GetBytesPerPixel(_format);
		assert(bpp != -1 && "Unsupported image format");

		VkDeviceSize imageSize = _width * _height * bpp * _layerCount;

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
		imageInfo.mipLevels = _mipLevels;
		imageInfo.arrayLayers = _layerCount;
		imageInfo.format = _format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = _flags;

		// VMA_MEMORY_USAGE_GPU_ONLY - Check internal usage for VmaAllocationCreateInfo
		m_device.CreateImageWithInfo(imageInfo, _image, _allocation);

		// Copy data from staging buffer to Vulkan image
		VkCommandBuffer commandBuffer = m_device.BeginSingleTimeCommands();

		m_device.TransitionImageLayout(commandBuffer, _image, _format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, _layerCount, _mipLevels);
		m_device.CopyBufferToImage(commandBuffer, stagingBuffer.Buffer, _image, _width, _height, _layerCount, _mipLevels);
		m_device.TransitionImageLayout(commandBuffer, _image, _format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, _layerCount, _mipLevels);

		m_device.EndSingleTimeCommands(commandBuffer);

		// Cleanup staging buffer
		m_buffer->Destroy(stagingBuffer);
	}

	// nullptr does not have a deduced type, so cannot specialize the template, so I made an overload, but I keep in the header for reference!
	void CreateTextureImage(const std::nullptr_t*, int32 width, int32 height, VkFormat format, VkImage& image, VmaAllocation& allocation,
		uint32 layerCount, uint32 mipLevels, VkImageCreateFlags flags)
	{
		// Create the Vulkan image
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32>(width);
		imageInfo.extent.height = static_cast<uint32>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = layerCount;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // Allow rendering and copying into this image
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = flags;

		m_device.CreateImageWithInfo(imageInfo, image, allocation);

		// Transition the image layout to be ready for rendering
		VkCommandBuffer commandBuffer = m_device.BeginSingleTimeCommands();
		m_device.TransitionImageLayout(commandBuffer, image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, layerCount, mipLevels);
		m_device.EndSingleTimeCommands(commandBuffer);
	}

	VkImageView CreateImageView(VkImage _image, VkFormat _format, uint32 _layerCount = 1, uint32 _mipLevels = 1);
	VkSampler CreateTextureSampler();

private:
	VesperApp& m_app;
	Device& m_device;
	std::unique_ptr<Buffer> m_buffer;
	std::vector<std::shared_ptr<TextureData>> m_textures;
	std::unordered_map<uint32, int32> m_textureLookup;
};

VESPERENGINE_NAMESPACE_END
