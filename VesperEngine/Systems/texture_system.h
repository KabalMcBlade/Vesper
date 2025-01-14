// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\texture_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once


#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/model_data.h"

#include "App/vesper_app.h"

#include <unordered_map>
#include <memory>
#include <array>


VESPERENGINE_NAMESPACE_BEGIN

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

	// PBR default textures
	static const DefaultTextureType RoughnessTexture;
	static const DefaultTextureType MetallicTexture;
	static const DefaultTextureType SheenTexture;
	static const DefaultTextureType EmissiveTexture;


public:
	TextureSystem(Device& _device);
	~TextureSystem() = default;

	TextureSystem(const TextureSystem&) = delete;
	TextureSystem& operator=(const TextureSystem&) = delete;

	// if _overrideFormat is different from VK_FORMAT_UNDEFINED, it uses whatever has been passed, 
	// otherwise use channels definition to figure out what format to use
	std::shared_ptr<TextureData> LoadTexture(const std::string& _path, VkFormat _overrideFormat = VK_FORMAT_UNDEFINED);
	std::shared_ptr<TextureData> LoadTexture(const std::string& _name, VkFormat _format, const uint8* _data, int32 _width, int32 _height);
	std::shared_ptr<TextureData> LoadTexture(const DefaultTextureType& _defaultTexture);
	std::shared_ptr<TextureData> LoadCubemap(const std::array<std::string, 6>& _paths, VkFormat _overrideFormat = VK_FORMAT_UNDEFINED);
	void Cleanup();

	std::shared_ptr<TextureData> GenerateOrLoadBRDFLutTexture(VesperApp& _app, const std::string& _saveLoadPath, VkExtent2D _extent);

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
	void CreateTextureImage(const uint8* _data, int32 _width, int32 _height, VkFormat _format, VkImage& _image, VmaAllocation& _allocation,
							uint32 _layerCount = 1, uint32 _mipLevels = 1, VkImageCreateFlags _flags = 0);
	VkImageView CreateImageView(VkImage _image, VkFormat _format, uint32 _layerCount = 1, uint32 _mipLevels = 1);
	VkSampler CreateTextureSampler();

private:
	Device& m_device;
	std::unique_ptr<Buffer> m_buffer;
	std::vector<std::shared_ptr<TextureData>> m_textures;
	std::unordered_map<uint32, int32> m_textureLookup;
};

VESPERENGINE_NAMESPACE_END
