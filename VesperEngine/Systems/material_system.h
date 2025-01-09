// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\material_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/model_data.h"

#include "Components/graphics_components.h"

#include "ECS/ECS/ecs.h"

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;

class VESPERENGINE_API MaterialSystem final
{
public:
	MaterialSystem(VesperApp& _app, Device& _device);
	~MaterialSystem() = default;

	MaterialSystem(const MaterialSystem&) = delete;
	MaterialSystem& operator=(const MaterialSystem&) = delete;

public:
	static std::unique_ptr<MaterialData> CreateDefaultPhongMaterialData();

public:
	int32 CreateMaterial(const MaterialData& _data);

	VESPERENGINE_INLINE std::shared_ptr<MaterialData> GetMaterial(int32 _index)
	{
		if (_index >= 0 && _index < static_cast<int32>(m_materials.size()))
		{
			return m_materials[_index];
		}

		throw std::out_of_range("Invalid material index");
	}

	void Cleanup();

	// _path is where to load and to save in case does not yet exist.
	void GenerateOrLoadBRDFLutTexture(const std::string& _path, VkExtent2D _extent);

private:
	int32 CreatePhongMaterial(const MaterialData& _data);
	int32 CreatePBRMaterial(const MaterialData& _data);

	// if _overrideFormat is different from VK_FORMAT_UNDEFINED, it uses whatever has been passed, otherwise use channels definition
	// to figure out what format to use
	TextureData LoadTexture(const std::string& _path, VkFormat _overrideFormat = VK_FORMAT_UNDEFINED);
	uint8* LoadTextureData(const std::string& _path, int32& _width, int32& _height, int32& _channels, int32 _desired_channels);
	void FreeTextureData(uint8* _data);
	void CreateTextureImage(const uint8* _data, int32 _width, int32 _height, VkFormat _format, VkImage& _image, VmaAllocation& _allocation);
	VkImageView CreateImageView(VkImage _image, VkFormat _format);
	VkSampler CreateTextureSampler();

	void CreateDefaultTexturesCommon();
	void CreateDefaultTexturesPhong();
	void CreateDefaultTexturesPBR();
	TextureData LoadDefaultTexture(const uint8* _data, int32 _width, int32 _height);

	void DestroyTextureIfUnused(TextureData& _texture);
	void DestroyUniformBufferIfUnused(BufferComponent& _buffer);

private:
	VesperApp& m_app;
	Device& m_device;
	std::unique_ptr<Buffer> m_buffer;

	std::vector<std::shared_ptr<MaterialData>> m_materials;
	std::unordered_map<size_t, int32> m_materialLookup;
	std::unordered_map<VkImage, int32> m_textureReferenceCount;
	std::unordered_map<VkBuffer, int32> m_uniformBufferReferenceCount;

private:	
	// Phong default textures
	TextureData m_defaultDiffuseTexture;
	TextureData m_defaultSpecularTexture;
	TextureData m_defaultNormalTexture;
	TextureData m_defaultAmbientTexture;

	// PBR default textures
	TextureData m_defaultRoughnessTexture;
	TextureData m_defaultMetallicTexture;
	TextureData m_defaultSheenTexture;
	TextureData m_defaultEmissiveTexture;
	TextureData m_defaultNormalMapTexture;

	// Generated/loaded textures
	TextureData m_brdfLutTexture;
};

/*
int32 materialIndex = materialSystem->CreateMaterial(materialData);
auto material = materialSystem->GetMaterial(materialIndex);

// Use the material...
if (material->Type == MaterialType::Phong)
{
	auto phongMaterial = std::dynamic_pointer_cast<PhongMaterial>(material);
	// Bind Phong material...
} else if (material->Type == MaterialType::PBR) 
{
	auto pbrMaterial = std::dynamic_pointer_cast<PBRMaterial>(material);
	// Bind PBR material...
}
*/

VESPERENGINE_NAMESPACE_END
