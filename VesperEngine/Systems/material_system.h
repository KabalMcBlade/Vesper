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

private:
	int32 CreatePhongMaterial(const MaterialData& _data);
	int32 CreatePBRMaterial(const MaterialData& _data);

	TextureData LoadTexture(const std::string& _path);
	uint8* LoadTextureData(const std::string& _path, int32& _width, int32& _height, int32& _channels, int32 _desired_channels);
	void FreeTextureData(uint8* _data);
	void CreateTextureImage(const uint8* _data, int32 _width, int32 _height, VkFormat _format, VkImage& _image, VmaAllocation& _allocation);
	VkImageView CreateImageView(VkImage _image, VkFormat _format);
	VkSampler CreateTextureSampler();

	void CreateDefaultTexturesPhong();
	void CreateDefaultTexturesPBR();
	TextureData LoadDefaultTexture(const uint8* _data, int32 _width, int32 _height);

private:
	VesperApp& m_app;
	Device& m_device;
	std::unique_ptr<Buffer> m_buffer;

	std::vector<std::shared_ptr<MaterialData>> m_materials;
	std::unordered_map<size_t, int32> m_materialLookup;

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
