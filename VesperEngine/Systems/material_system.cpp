#include "material_system.h"

#include "Utility/hash.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "App/vesper_app.h"

#include "../stb/stb_image.h"


VESPERENGINE_NAMESPACE_BEGIN

bool HasExtension(const std::string& filename) 
{
	// Find the last occurrence of the directory separator
	size_t lastSlash = filename.find_last_of("/\\");
	// Find the last occurrence of a period
	size_t lastDot = filename.find_last_of('.');

	// Check if the period comes after the last slash (if any)
	return (lastDot != std::string::npos) && (lastDot > lastSlash);
}

size_t HashMaterialData(const MaterialData& _data) 
{
	size_t hash = 0;

	auto hashCombine = [&hash](const auto& value)
	{
		vesper::HashCombine(hash, value);
	};

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

MaterialSystem::MaterialSystem(VesperApp& _app, Device& _device)
	: m_app(_app)
	, m_device(_device)
{
	m_buffer = std::make_unique<Buffer>(m_device);

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

		phongMaterial->Shininess = phongData->Shininess;
		phongMaterial->AmbientColor = phongData->AmbientColor;
		phongMaterial->DiffuseColor = phongData->DiffuseColor;
		phongMaterial->SpecularColor = phongData->SpecularColor;
		phongMaterial->EmissionColor = phongData->EmissionColor;

		// Load textures
		if (!phongData->AmbientTexturePath.empty() && HasExtension(phongData->AmbientTexturePath))
		{
			phongMaterial->AmbientTexture = LoadTexture(phongData->AmbientTexturePath);
		}
		if (!phongData->DiffuseTexturePath.empty() && HasExtension(phongData->DiffuseTexturePath))
		{
			phongMaterial->DiffuseTexture = LoadTexture(phongData->DiffuseTexturePath);
		}
		if (!phongData->SpecularTexturePath.empty() && HasExtension(phongData->SpecularTexturePath))
		{
			phongMaterial->SpecularTexture = LoadTexture(phongData->SpecularTexturePath);
		}
		if (!phongData->NormalTexturePath.empty() && HasExtension(phongData->NormalTexturePath))
		{
			phongMaterial->SpecularTexture = LoadTexture(phongData->NormalTexturePath);
		}

		// Load default textures if missing
		if (phongMaterial->AmbientTexture.AllocationMemory == VK_NULL_HANDLE)
		{
			phongMaterial->AmbientTexture = m_defaultAmbientTexture;
		}
		if (phongMaterial->DiffuseTexture.AllocationMemory == VK_NULL_HANDLE)
		{
			phongMaterial->DiffuseTexture = m_defaultDiffuseTexture;
		}
		if (phongMaterial->SpecularTexture.AllocationMemory == VK_NULL_HANDLE)
		{
			phongMaterial->SpecularTexture = m_defaultSpecularTexture;
		}
		if (phongMaterial->NormalTexture.AllocationMemory == VK_NULL_HANDLE)
		{
			phongMaterial->NormalTexture = m_defaultNormalTexture;
		}

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

		pbrMaterial->Roughness = pbrData->Roughness;
		pbrMaterial->Metallic = pbrData->Metallic;
		pbrMaterial->Sheen = pbrData->Sheen;
		pbrMaterial->ClearcoatThickness = pbrData->ClearcoatThickness;
		pbrMaterial->ClearcoatRoughness = pbrData->ClearcoatRoughness;
		pbrMaterial->Anisotropy = pbrData->Anisotropy;
		pbrMaterial->AnisotropyRotation = pbrData->AnisotropyRotation;

		// Load textures
		if (!pbrData->RoughnessTexturePath.empty() && HasExtension(pbrData->RoughnessTexturePath))
		{
			pbrMaterial->RoughnessTexture = LoadTexture(pbrData->RoughnessTexturePath);
		}
		if (!pbrData->MetallicTexturePath.empty() && HasExtension(pbrData->MetallicTexturePath))
		{
			pbrMaterial->MetallicTexture = LoadTexture(pbrData->MetallicTexturePath);
		}
		if (!pbrData->SheenTexturePath.empty() && HasExtension(pbrData->SheenTexturePath))
		{
			pbrMaterial->SheenTexture = LoadTexture(pbrData->SheenTexturePath);
		}
		if (!pbrData->EmissiveTexturePath.empty() && HasExtension(pbrData->EmissiveTexturePath))
		{
			pbrMaterial->EmissiveTexture = LoadTexture(pbrData->EmissiveTexturePath);
		}
		if (!pbrData->NormalMapTexturePath.empty() && HasExtension(pbrData->NormalMapTexturePath))
		{
			pbrMaterial->NormalMapTexture = LoadTexture(pbrData->NormalMapTexturePath);
		}

		// Load default textures if missing
		if (pbrMaterial->RoughnessTexture.AllocationMemory == VK_NULL_HANDLE)
		{
			pbrMaterial->RoughnessTexture = m_defaultRoughnessTexture;
		}
		if (pbrMaterial->MetallicTexture.AllocationMemory == VK_NULL_HANDLE)
		{
			pbrMaterial->MetallicTexture = m_defaultMetallicTexture;
		}
		if (pbrMaterial->SheenTexture.AllocationMemory == VK_NULL_HANDLE)
		{
			pbrMaterial->SheenTexture = m_defaultSheenTexture;
		}
		if (pbrMaterial->EmissiveTexture.AllocationMemory == VK_NULL_HANDLE)
		{
			pbrMaterial->EmissiveTexture = m_defaultEmissiveTexture;
		}
		if (pbrMaterial->NormalMapTexture.AllocationMemory == VK_NULL_HANDLE)
		{
			pbrMaterial->NormalMapTexture = m_defaultNormalMapTexture;
		}

		m_materials.push_back(pbrMaterial);
		return static_cast<int32>(m_materials.size() - 1);
	}
	else
	{
		throw std::runtime_error("Material passed is described as PBR but does not contain PBR data");
	}
}


void MaterialSystem::Cleanup() 
{
	for (const auto& material : m_materials) 
	{
		if (material->Type == MaterialType::Phong) 
		{
			auto phongMaterial = std::dynamic_pointer_cast<MaterialPhong>(material);

			if (phongMaterial->AmbientTexture.Image != VK_NULL_HANDLE) 
			{
				vkDestroyImageView(m_device.GetDevice(), phongMaterial->AmbientTexture.ImageView, nullptr);
				vkDestroySampler(m_device.GetDevice(), phongMaterial->AmbientTexture.Sampler, nullptr);
				vmaDestroyImage(m_device.GetAllocator(), phongMaterial->AmbientTexture.Image, phongMaterial->AmbientTexture.AllocationMemory);
			}

			if (phongMaterial->DiffuseTexture.Image != VK_NULL_HANDLE) 
			{
				vkDestroyImageView(m_device.GetDevice(), phongMaterial->DiffuseTexture.ImageView, nullptr);
				vkDestroySampler(m_device.GetDevice(), phongMaterial->DiffuseTexture.Sampler, nullptr);
				vmaDestroyImage(m_device.GetAllocator(), phongMaterial->DiffuseTexture.Image, phongMaterial->DiffuseTexture.AllocationMemory);
			}

			if (phongMaterial->NormalTexture.Image != VK_NULL_HANDLE)
			{
				vkDestroyImageView(m_device.GetDevice(), phongMaterial->NormalTexture.ImageView, nullptr);
				vkDestroySampler(m_device.GetDevice(), phongMaterial->NormalTexture.Sampler, nullptr);
				vmaDestroyImage(m_device.GetAllocator(), phongMaterial->NormalTexture.Image, phongMaterial->NormalTexture.AllocationMemory);
			}

			if (phongMaterial->SpecularTexture.Image != VK_NULL_HANDLE) 
			{
				vkDestroyImageView(m_device.GetDevice(), phongMaterial->SpecularTexture.ImageView, nullptr);
				vkDestroySampler(m_device.GetDevice(), phongMaterial->SpecularTexture.Sampler, nullptr);
				vmaDestroyImage(m_device.GetAllocator(), phongMaterial->SpecularTexture.Image, phongMaterial->SpecularTexture.AllocationMemory);
			}
		}
		else if (material->Type == MaterialType::PBR) 
		{
			auto pbrMaterial = std::dynamic_pointer_cast<MaterialPBR>(material);

			if (pbrMaterial->RoughnessTexture.Image != VK_NULL_HANDLE) 
			{
				vkDestroyImageView(m_device.GetDevice(), pbrMaterial->RoughnessTexture.ImageView, nullptr);
				vkDestroySampler(m_device.GetDevice(), pbrMaterial->RoughnessTexture.Sampler, nullptr);
				vmaDestroyImage(m_device.GetAllocator(), pbrMaterial->RoughnessTexture.Image, pbrMaterial->RoughnessTexture.AllocationMemory);
			}

			if (pbrMaterial->MetallicTexture.Image != VK_NULL_HANDLE) 
			{
				vkDestroyImageView(m_device.GetDevice(), pbrMaterial->MetallicTexture.ImageView, nullptr);
				vkDestroySampler(m_device.GetDevice(), pbrMaterial->MetallicTexture.Sampler, nullptr);
				vmaDestroyImage(m_device.GetAllocator(), pbrMaterial->MetallicTexture.Image, pbrMaterial->MetallicTexture.AllocationMemory);
			}

			if (pbrMaterial->EmissiveTexture.Image != VK_NULL_HANDLE) 
			{
				vkDestroyImageView(m_device.GetDevice(), pbrMaterial->EmissiveTexture.ImageView, nullptr);
				vkDestroySampler(m_device.GetDevice(), pbrMaterial->EmissiveTexture.Sampler, nullptr);
				vmaDestroyImage(m_device.GetAllocator(), pbrMaterial->EmissiveTexture.Image, pbrMaterial->EmissiveTexture.AllocationMemory);
			}

			if (pbrMaterial->NormalMapTexture.Image != VK_NULL_HANDLE) 
			{
				vkDestroyImageView(m_device.GetDevice(), pbrMaterial->NormalMapTexture.ImageView, nullptr);
				vkDestroySampler(m_device.GetDevice(), pbrMaterial->NormalMapTexture.Sampler, nullptr);
				vmaDestroyImage(m_device.GetAllocator(), pbrMaterial->NormalMapTexture.Image, pbrMaterial->NormalMapTexture.AllocationMemory);
			}
		}
	}


	// Destroy default textures - Phong
	vkDestroyImageView(m_device.GetDevice(), m_defaultDiffuseTexture.ImageView, nullptr);
	vkDestroySampler(m_device.GetDevice(), m_defaultDiffuseTexture.Sampler, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_defaultDiffuseTexture.Image, m_defaultDiffuseTexture.AllocationMemory);

	vkDestroyImageView(m_device.GetDevice(), m_defaultSpecularTexture.ImageView, nullptr);
	vkDestroySampler(m_device.GetDevice(), m_defaultSpecularTexture.Sampler, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_defaultSpecularTexture.Image, m_defaultSpecularTexture.AllocationMemory);

	vkDestroyImageView(m_device.GetDevice(), m_defaultNormalTexture.ImageView, nullptr);
	vkDestroySampler(m_device.GetDevice(), m_defaultNormalTexture.Sampler, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_defaultNormalTexture.Image, m_defaultNormalTexture.AllocationMemory);

	vkDestroyImageView(m_device.GetDevice(), m_defaultAmbientTexture.ImageView, nullptr);
	vkDestroySampler(m_device.GetDevice(), m_defaultAmbientTexture.Sampler, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_defaultAmbientTexture.Image, m_defaultAmbientTexture.AllocationMemory);


	// Destroy default textures - PBR
	vkDestroyImageView(m_device.GetDevice(), m_defaultRoughnessTexture.ImageView, nullptr);
	vkDestroySampler(m_device.GetDevice(), m_defaultRoughnessTexture.Sampler, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_defaultRoughnessTexture.Image, m_defaultRoughnessTexture.AllocationMemory);

	vkDestroyImageView(m_device.GetDevice(), m_defaultMetallicTexture.ImageView, nullptr);
	vkDestroySampler(m_device.GetDevice(), m_defaultMetallicTexture.Sampler, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_defaultMetallicTexture.Image, m_defaultMetallicTexture.AllocationMemory);

	vkDestroyImageView(m_device.GetDevice(), m_defaultSheenTexture.ImageView, nullptr);
	vkDestroySampler(m_device.GetDevice(), m_defaultSheenTexture.Sampler, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_defaultSheenTexture.Image, m_defaultSheenTexture.AllocationMemory);

	vkDestroyImageView(m_device.GetDevice(), m_defaultEmissiveTexture.ImageView, nullptr);
	vkDestroySampler(m_device.GetDevice(), m_defaultEmissiveTexture.Sampler, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_defaultEmissiveTexture.Image, m_defaultEmissiveTexture.AllocationMemory);

	vkDestroyImageView(m_device.GetDevice(), m_defaultNormalMapTexture.ImageView, nullptr);
	vkDestroySampler(m_device.GetDevice(), m_defaultNormalMapTexture.Sampler, nullptr);
	vmaDestroyImage(m_device.GetAllocator(), m_defaultNormalMapTexture.Image, m_defaultNormalMapTexture.AllocationMemory);

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

	void* bufferData;
	m_buffer->Map(stagingBuffer, &bufferData);
	m_buffer->WriteToBuffer(bufferData, (void*)_data, static_cast<size_t>(imageSize));
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
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
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


void MaterialSystem::CreateDefaultTexturesPhong()
{
	// White diffuse texture
	uint8 whiteData[4] = { 255, 255, 255, 255 }; // RGBA
	m_defaultDiffuseTexture = LoadDefaultTexture(whiteData, 1, 1);

	// Black specular texture
	uint8 blackData[4] = { 0, 0, 0, 255 }; // RGBA
	m_defaultSpecularTexture = LoadDefaultTexture(blackData, 1, 1);

	// Flat normal texture
	uint8 normalData[4] = { 128, 128, 255, 255 }; // RGBA
	m_defaultNormalTexture = LoadDefaultTexture(normalData, 1, 1);

	// Grey ambient texture
	uint8 greyData[4] = { 128, 128, 128, 255 }; // RGBA (50% grey)
	m_defaultAmbientTexture = LoadDefaultTexture(greyData, 1, 1);
}

void MaterialSystem::CreateDefaultTexturesPBR()
{
	// Default Roughness (White - fully rough)
	uint8 whiteRoughnessData[4] = { 255, 255, 255, 255 }; // RGBA
	m_defaultRoughnessTexture = LoadDefaultTexture(whiteRoughnessData, 1, 1);

	// Default Metallic (Black - non-metallic)
	uint8 blackMetallicData[4] = { 0, 0, 0, 255 }; // RGBA
	m_defaultMetallicTexture = LoadDefaultTexture(blackMetallicData, 1, 1);

	// Default Sheen (Grey - 50%)
	uint8 greySheenData[4] = { 128, 128, 128, 255 }; // RGBA
	m_defaultSheenTexture = LoadDefaultTexture(greySheenData, 1, 1);

	// Default Emissive (Black - no emissive lighting)
	uint8 blackEmissiveData[4] = { 0, 0, 0, 255 }; // RGBA
	m_defaultEmissiveTexture = LoadDefaultTexture(blackEmissiveData, 1, 1);

	// Default Normal Map (Flat normal)
	uint8 flatNormalData[4] = { 128, 128, 255, 255 }; // RGBA
	m_defaultNormalMapTexture = LoadDefaultTexture(flatNormalData, 1, 1);
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
