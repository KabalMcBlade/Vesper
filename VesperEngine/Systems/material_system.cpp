#include "material_system.h"

#include "Utility/hash.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "App/vesper_app.h"
#include "App/file_system.h"

#include "Utility/logger.h"

#include "../stb/stb_image.h"
#include <random>


VESPERENGINE_NAMESPACE_BEGIN

size_t HashMaterialData(const MaterialData& _data) 
{
	size_t hash = 0;

	auto hashCombine = [&hash](const auto& value)
	{
		vesper::HashCombine(hash, value);
	};

	hashCombine(_data.Name);
	hashCombine(_data.Type);

	if (_data.Type == MaterialType::Phong)
	{
		const auto* phong = dynamic_cast<const MaterialDataPhong*>(&_data);
		if (phong)
		{
			hashCombine(phong->AmbientColor);
			hashCombine(phong->DiffuseColor);
			hashCombine(phong->SpecularColor);
			hashCombine(phong->EmissionColor);
			hashCombine(phong->Shininess);
			hashCombine(phong->AmbientTexturePath);
			hashCombine(phong->DiffuseTexturePath);
			hashCombine(phong->SpecularTexturePath);
		}
	}
	else if (_data.Type == MaterialType::PBR)
	{
		const auto* pbr = dynamic_cast<const MaterialDataPBR*>(&_data);
		if (pbr)
		{
			hashCombine(pbr->Roughness);
			hashCombine(pbr->Metallic);
			hashCombine(pbr->Sheen);
			hashCombine(pbr->ClearcoatThickness);
			hashCombine(pbr->ClearcoatRoughness);
			hashCombine(pbr->Anisotropy);
			hashCombine(pbr->AnisotropyRotation);
			hashCombine(pbr->RoughnessTexturePath);
			hashCombine(pbr->MetallicTexturePath);
			hashCombine(pbr->SheenTexturePath);
			hashCombine(pbr->EmissiveTexturePath);
			hashCombine(pbr->NormalMapTexturePath);
		}
	}

	return hash;
}


// default material is Phong, with default texture and values
std::unique_ptr<MaterialData> MaterialSystem::CreateDefaultMaterialData()
{
	auto defaultMaterial = std::make_unique<MaterialDataPhong>();
	defaultMaterial->Type = MaterialType::Phong;
	defaultMaterial->Name = "DefaultMaterial";

	defaultMaterial->AmbientColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	defaultMaterial->DiffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	defaultMaterial->SpecularColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	defaultMaterial->EmissionColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	return defaultMaterial;
}


MaterialSystem::MaterialSystem(VesperApp& _app, Device& _device)
	: m_app(_app)
	, m_device(_device)
{
	m_buffer = std::make_unique<Buffer>(m_device);

	CreateDefaultTexturesCommon();
	CreateDefaultTexturesPhong();
	CreateDefaultTexturesPBR();
}

int32 MaterialSystem::CreateMaterial(const MaterialData& _data) 
{
	const size_t hash = HashMaterialData(_data);

	// Check if the material already exists
	auto it = m_materialLookup.find(hash);
	if (it != m_materialLookup.end())
	{
		return it->second; // Return existing material index
	}

	// Create a new material based on its type
	int32 materialIndex;
	if (_data.Type == MaterialType::Phong)
	{
		materialIndex = CreatePhongMaterial(_data);
	}
	else if (_data.Type == MaterialType::PBR)
	{
		materialIndex = CreatePBRMaterial(_data);
	}
	else
	{
		throw std::runtime_error("Unsupported material type");
	}

	// Store the new material in the lookup map
	m_materialLookup[hash] = materialIndex;
	return materialIndex;
}

int32 MaterialSystem::CreatePhongMaterial(const MaterialData& _data)
{
	const auto* phongData = dynamic_cast<const MaterialDataPhong*>(&_data);
	if (phongData)
	{
		auto phongMaterial = std::make_shared<MaterialPhong>();

		const uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);

		MaterialPhongValues phongValues;
		phongValues.TextureFlags = 0;	// 0: HasAmbientTexture, 1: HasDiffuseTexture, 2: HasSpecularTexture, 3: HasNormalTexture

		// Load textures
		if (!phongData->AmbientTexturePath.empty() && FileSystem::HasExtension(phongData->AmbientTexturePath))
		{
			phongValues.TextureFlags |= (1 << 0);
			phongMaterial->AmbientTexture = LoadTexture(phongData->AmbientTexturePath);
			++m_textureReferenceCount[phongMaterial->AmbientTexture.Image];
		}
		if (!phongData->DiffuseTexturePath.empty() && FileSystem::HasExtension(phongData->DiffuseTexturePath))
		{
			phongValues.TextureFlags |= (1 << 1);
			phongMaterial->DiffuseTexture = LoadTexture(phongData->DiffuseTexturePath);
			++m_textureReferenceCount[phongMaterial->DiffuseTexture.Image];
		}
		if (!phongData->SpecularTexturePath.empty() && FileSystem::HasExtension(phongData->SpecularTexturePath))
		{
			phongValues.TextureFlags |= (1 << 2);
			phongMaterial->SpecularTexture = LoadTexture(phongData->SpecularTexturePath);
			++m_textureReferenceCount[phongMaterial->SpecularTexture.Image];
		}
		if (!phongData->NormalTexturePath.empty() && FileSystem::HasExtension(phongData->NormalTexturePath))
		{
			phongValues.TextureFlags |= (1 << 3);
			phongMaterial->NormalTexture = LoadTexture(phongData->NormalTexturePath);
			++m_textureReferenceCount[phongMaterial->NormalTexture.Image];
		}

		// Load default textures if missing (only to avoid custom descriptors binding)
		if (phongMaterial->AmbientTexture.Image == VK_NULL_HANDLE)
		{
			phongMaterial->AmbientTexture = m_defaultAmbientTexture;
			++m_textureReferenceCount[phongMaterial->AmbientTexture.Image];
		}
		if (phongMaterial->DiffuseTexture.Image == VK_NULL_HANDLE)
		{
			phongMaterial->DiffuseTexture = m_defaultDiffuseTexture;
			++m_textureReferenceCount[phongMaterial->DiffuseTexture.Image];
		}
		if (phongMaterial->SpecularTexture.Image == VK_NULL_HANDLE)
		{
			phongMaterial->SpecularTexture = m_defaultSpecularTexture;
			++m_textureReferenceCount[phongMaterial->SpecularTexture.Image];
		}
		if (phongMaterial->NormalTexture.Image == VK_NULL_HANDLE)
		{
			phongMaterial->NormalTexture = m_defaultNormalTexture;
			++m_textureReferenceCount[phongMaterial->NormalTexture.Image];
		}

		// Load values in Uniform buffer
		phongMaterial->UniformBuffer = m_buffer->Create<BufferComponent>(
			sizeof(MaterialPhongValues),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			minUboAlignment,
			true
		);

		phongValues.Shininess = phongData->Shininess;
		phongValues.AmbientColor = phongData->AmbientColor;
		phongValues.DiffuseColor = phongData->DiffuseColor;
		phongValues.SpecularColor = phongData->SpecularColor;
		phongValues.EmissionColor = phongData->EmissionColor;

		phongMaterial->UniformBuffer.MappedMemory = &phongValues;
		m_buffer->WriteToBuffer(phongMaterial->UniformBuffer);

		++m_uniformBufferReferenceCount[phongMaterial->UniformBuffer.Buffer];

		m_materials.push_back(phongMaterial);
		return static_cast<int32>(m_materials.size() - 1);
	}
	else
	{
		throw std::runtime_error("Material passed is described as phong but does not contain phong data");
	}
}

int32 MaterialSystem::CreatePBRMaterial(const MaterialData& _data)
{
	const auto* pbrData = dynamic_cast<const MaterialDataPBR*>(&_data);
	if (pbrData)
	{
		auto pbrMaterial = std::make_shared<MaterialPBR>();

		const uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);

		MaterialPBRValues pbrValues;
		pbrValues.TextureFlags = 0;   // 0: HasRoughnessTexture, 1: HasMetallicTexture, 2: HasSheenTexture, 3: HasEmissiveTexture, 4: NormalMapTexture

		// Load textures
		if (!pbrData->RoughnessTexturePath.empty() && FileSystem::HasExtension(pbrData->RoughnessTexturePath))
		{
			pbrValues.TextureFlags |= (1 << 0);
			pbrMaterial->RoughnessTexture = LoadTexture(pbrData->RoughnessTexturePath);
			++m_textureReferenceCount[pbrMaterial->RoughnessTexture.Image];
		}
		if (!pbrData->MetallicTexturePath.empty() && FileSystem::HasExtension(pbrData->MetallicTexturePath))
		{
			pbrValues.TextureFlags |= (1 << 1);
			pbrMaterial->MetallicTexture = LoadTexture(pbrData->MetallicTexturePath);
			++m_textureReferenceCount[pbrMaterial->MetallicTexture.Image];
		}
		if (!pbrData->SheenTexturePath.empty() && FileSystem::HasExtension(pbrData->SheenTexturePath))
		{
			pbrValues.TextureFlags |= (1 << 2);
			pbrMaterial->SheenTexture = LoadTexture(pbrData->SheenTexturePath);
			++m_textureReferenceCount[pbrMaterial->SheenTexture.Image];
		}
		if (!pbrData->EmissiveTexturePath.empty() && FileSystem::HasExtension(pbrData->EmissiveTexturePath))
		{
			pbrValues.TextureFlags |= (1 << 3);
			pbrMaterial->EmissiveTexture = LoadTexture(pbrData->EmissiveTexturePath);
			++m_textureReferenceCount[pbrMaterial->EmissiveTexture.Image];
		}
		if (!pbrData->NormalMapTexturePath.empty() && FileSystem::HasExtension(pbrData->NormalMapTexturePath))
		{
			pbrValues.TextureFlags |= (1 << 4);
			pbrMaterial->NormalMapTexture = LoadTexture(pbrData->NormalMapTexturePath);
			++m_textureReferenceCount[pbrMaterial->NormalMapTexture.Image];
		}

		// Load default textures if missing (only to avoid custom descriptors binding)
		if (pbrMaterial->RoughnessTexture.Image == VK_NULL_HANDLE)
		{
			pbrMaterial->RoughnessTexture = m_defaultRoughnessTexture;
			++m_textureReferenceCount[pbrMaterial->RoughnessTexture.Image];
		}
		if (pbrMaterial->MetallicTexture.Image == VK_NULL_HANDLE)
		{
			pbrMaterial->MetallicTexture = m_defaultMetallicTexture;
			++m_textureReferenceCount[pbrMaterial->MetallicTexture.Image];
		}
		if (pbrMaterial->SheenTexture.Image == VK_NULL_HANDLE)
		{
			pbrMaterial->SheenTexture = m_defaultSheenTexture;
			++m_textureReferenceCount[pbrMaterial->SheenTexture.Image];
		}
		if (pbrMaterial->EmissiveTexture.Image == VK_NULL_HANDLE)
		{
			pbrMaterial->EmissiveTexture = m_defaultEmissiveTexture;
			++m_textureReferenceCount[pbrMaterial->EmissiveTexture.Image];
		}
		if (pbrMaterial->NormalMapTexture.Image == VK_NULL_HANDLE)
		{
			pbrMaterial->NormalMapTexture = m_defaultNormalTexture;
			++m_textureReferenceCount[pbrMaterial->NormalMapTexture.Image];
		}

		// Load values in Uniform buffer
		pbrMaterial->UniformBuffer = m_buffer->Create<BufferComponent>(
			sizeof(MaterialPBRValues),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			minUboAlignment,
			true
		);

		pbrValues.Roughness = pbrData->Roughness;
		pbrValues.Metallic = pbrData->Metallic;
		pbrValues.Sheen = pbrData->Sheen;
		pbrValues.ClearcoatThickness = pbrData->ClearcoatThickness;
		pbrValues.ClearcoatRoughness = pbrData->ClearcoatRoughness;
		pbrValues.Anisotropy = pbrData->Anisotropy;
		pbrValues.AnisotropyRotation = pbrData->AnisotropyRotation;

		pbrMaterial->UniformBuffer.MappedMemory = &pbrValues;
		m_buffer->WriteToBuffer(pbrMaterial->UniformBuffer);

		++m_uniformBufferReferenceCount[pbrMaterial->UniformBuffer.Buffer];

		m_materials.push_back(pbrMaterial);
		return static_cast<int32>(m_materials.size() - 1);
	}
	else
	{
		throw std::runtime_error("Material passed is described as PBR but does not contain PBR data");
	}
}

void MaterialSystem::DestroyTextureIfUnused(TextureData& _texture)
{
	if (--m_textureReferenceCount[_texture.Image] <= 0)
	{
		if (_texture.Image != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_device.GetDevice(), _texture.ImageView, nullptr);
			vkDestroySampler(m_device.GetDevice(), _texture.Sampler, nullptr);
			vmaDestroyImage(m_device.GetAllocator(), _texture.Image, _texture.AllocationMemory);
		}

		m_textureReferenceCount.erase(_texture.Image);
	}
}

void MaterialSystem::DestroyUniformBufferIfUnused(BufferComponent& _buffer)
{
	if (--m_uniformBufferReferenceCount[_buffer.Buffer] <= 0)
	{
		if (_buffer.Buffer != VK_NULL_HANDLE)
		{
			m_buffer->Destroy(_buffer);
		}

		m_uniformBufferReferenceCount.erase(_buffer.Buffer);
	}
}

void MaterialSystem::Cleanup() 
{
	for (const auto& material : m_materials) 
	{
		if (material->Type == MaterialType::Phong) 
		{
			auto phongMaterial = std::dynamic_pointer_cast<MaterialPhong>(material);

			DestroyTextureIfUnused(phongMaterial->AmbientTexture);
			DestroyTextureIfUnused(phongMaterial->DiffuseTexture);
			DestroyTextureIfUnused(phongMaterial->NormalTexture);
			DestroyTextureIfUnused(phongMaterial->SpecularTexture);
			DestroyUniformBufferIfUnused(phongMaterial->UniformBuffer);
		}
		else if (material->Type == MaterialType::PBR) 
		{
			auto pbrMaterial = std::dynamic_pointer_cast<MaterialPBR>(material);

			DestroyTextureIfUnused(pbrMaterial->RoughnessTexture);
			DestroyTextureIfUnused(pbrMaterial->MetallicTexture);
			DestroyTextureIfUnused(pbrMaterial->SheenTexture);
			DestroyTextureIfUnused(pbrMaterial->EmissiveTexture);
			DestroyTextureIfUnused(pbrMaterial->NormalMapTexture);
			DestroyUniformBufferIfUnused(pbrMaterial->UniformBuffer);
		}
	}

	// Destroy default texture - Commons
	DestroyTextureIfUnused(m_defaultNormalTexture);

	// Destroy default textures - Phong
	DestroyTextureIfUnused(m_defaultDiffuseTexture);
	DestroyTextureIfUnused(m_defaultSpecularTexture);
	DestroyTextureIfUnused(m_defaultAmbientTexture);


	// Destroy default textures - PBR
	DestroyTextureIfUnused(m_defaultRoughnessTexture);
	DestroyTextureIfUnused(m_defaultMetallicTexture);
	DestroyTextureIfUnused(m_defaultSheenTexture);
	DestroyTextureIfUnused(m_defaultEmissiveTexture);

	m_materials.clear();
	m_materialLookup.clear();
}

TextureData MaterialSystem::LoadTexture(const std::string& _path)
{
	TextureData textureComponent;

	// Step 1: Load texture data
	int32 width, height, channels;
	uint8* data = LoadTextureData(_path, width, height, channels, STBI_rgb_alpha);
	if (!data) 
	{
		throw std::runtime_error("Failed to load texture: " + _path);
	}

	// Step 2: Determine Vulkan format
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // Default to SRGB 4 channels
	if (channels == 1) 
	{
		format = VK_FORMAT_R8_UNORM; // Single channel (gray-scale)
	}
	else if (channels == 3) 
	{
		format = VK_FORMAT_R8G8B8A8_SRGB; // Convert RGB to RGBA
	}

	// Step 3: Create Vulkan texture image
	CreateTextureImage(data, width, height, format, textureComponent.Image, textureComponent.AllocationMemory);

	// Step 4: Create Vulkan image view
	textureComponent.ImageView = CreateImageView(textureComponent.Image, format);

	// Step 5: Create Vulkan texture sampler
	textureComponent.Sampler = CreateTextureSampler();

	// Free the loaded texture data
	FreeTextureData(data);

	return textureComponent;
}

uint8* MaterialSystem::LoadTextureData(const std::string& _path, int32& _width, int32& _height, int32& _channels, int32 _desired_channels)
{
	uint8* data = stbi_load(_path.c_str(), &_width, &_height, &_channels, _desired_channels);
	if (!data) 
	{
		throw std::runtime_error("Failed to load texture file: " + _path);
	}
	return data;
}

void MaterialSystem::FreeTextureData(uint8* _data)
{
	stbi_image_free(_data);
}

void MaterialSystem::CreateTextureImage(const uint8* _data, int32 _width, int32 _height, VkFormat _format, VkImage& _image, VmaAllocation& _allocation)
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
	imageInfo.extent.width = static_cast<uint32_t>(_width);
	imageInfo.extent.height = static_cast<uint32_t>(_height);
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
	m_device.TransitionImageLayout(_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	m_device.CopyBufferToImage(stagingBuffer.Buffer, _image, _width, _height, 1);
	m_device.TransitionImageLayout(_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Cleanup staging buffer
	m_buffer->Destroy(stagingBuffer);
}

VkImageView MaterialSystem::CreateImageView(VkImage _image, VkFormat _format)
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

VkSampler MaterialSystem::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16; // Adjust as per GPU capabilities
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	VkSampler sampler;
	if (vkCreateSampler(m_device.GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}

	return sampler;
}


void MaterialSystem::CreateDefaultTexturesCommon()
{
	// Fake normal texture
	uint8 flatGray[4] = { 128, 128, 255, 255 }; // RGBA
	m_defaultNormalTexture = LoadDefaultTexture(flatGray, 1, 1);
	++m_textureReferenceCount[m_defaultNormalTexture.Image];
}

void MaterialSystem::CreateDefaultTexturesPhong()
{
	// White diffuse texture
	uint8 whiteData[4] = { 255, 255, 255, 255 }; // RGBA
	m_defaultDiffuseTexture = LoadDefaultTexture(whiteData, 1, 1);
	++m_textureReferenceCount[m_defaultDiffuseTexture.Image];

	// Black specular texture
	uint8 blackData[4] = { 0, 0, 0, 255 }; // RGBA
	m_defaultSpecularTexture = LoadDefaultTexture(blackData, 1, 1);
	++m_textureReferenceCount[m_defaultSpecularTexture.Image];

	// Grey ambient texture
	uint8 greyData[4] = { 128, 128, 128, 255 }; // RGBA (50% grey)
	m_defaultAmbientTexture = LoadDefaultTexture(greyData, 1, 1);
	++m_textureReferenceCount[m_defaultAmbientTexture.Image];
}

void MaterialSystem::CreateDefaultTexturesPBR()
{
	// Default Roughness (White - fully rough)
	uint8 whiteRoughnessData[4] = { 255, 255, 255, 255 }; // RGBA
	m_defaultRoughnessTexture = LoadDefaultTexture(whiteRoughnessData, 1, 1);
	++m_textureReferenceCount[m_defaultRoughnessTexture.Image];

	// Default Metallic (Black - non-metallic)
	uint8 blackMetallicData[4] = { 0, 0, 0, 255 }; // RGBA
	m_defaultMetallicTexture = LoadDefaultTexture(blackMetallicData, 1, 1);
	++m_textureReferenceCount[m_defaultMetallicTexture.Image];

	// Default Sheen (Grey - 50%)
	uint8 greySheenData[4] = { 128, 128, 128, 255 }; // RGBA
	m_defaultSheenTexture = LoadDefaultTexture(greySheenData, 1, 1);
	++m_textureReferenceCount[m_defaultSheenTexture.Image];

	// Default Emissive (Black - no emissive lighting)
	uint8 blackEmissiveData[4] = { 0, 0, 0, 255 }; // RGBA
	m_defaultEmissiveTexture = LoadDefaultTexture(blackEmissiveData, 1, 1);
	++m_textureReferenceCount[m_defaultEmissiveTexture.Image];
}

TextureData MaterialSystem::LoadDefaultTexture(const uint8* _data, int32 _width, int32 _height)
{
	TextureData textureComponent;
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	CreateTextureImage(_data, _width, _height, format, textureComponent.Image, textureComponent.AllocationMemory);
	textureComponent.ImageView = CreateImageView(textureComponent.Image, format);
	textureComponent.Sampler = CreateTextureSampler();

	return textureComponent;
}

VESPERENGINE_NAMESPACE_END
