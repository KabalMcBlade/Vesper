// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\texture_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/texture_system.h"
#include "Systems/brdf_lut_generation_system.h"
#include "Systems/irradiance_convolution_generation_system.h"
#include "Systems/pre_filtered_environment_generation_system.h"

#include "Backend/offscreen_renderer.h"
#include "Backend/device.h"

#include "App/vesper_app.h"
#include "App/file_system.h"

#include "Utility/hash.h"
#include "Utility/hdr_cubemap_generation.h"
#include "Core/glm_config.h"

#include "ThirdParty/include/stb/stb_image.h"

#include <filesystem>
#include <algorithm>
#include <vector>


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

const TextureSystem::DefaultTextureType TextureSystem::AlphaTexture =
{
        "_AlphaTexture_",
        { 255, 255, 255, 255 },
        VK_FORMAT_R8G8B8A8_UNORM,
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

const TextureSystem::DefaultTextureType TextureSystem::BaseColorTexture =
{
	"_BaseColorTexture_",
	{255, 255, 255, 255},
	VK_FORMAT_R8G8B8A8_SRGB,
	1, 1 };

const TextureSystem::DefaultTextureType TextureSystem::AOTexture = 
{
	"_AOTexture_",
	{255, 255, 255, 255},
	VK_FORMAT_R8G8B8A8_UNORM,
	1, 1 };

int32 TextureSystem::GetBytesPerPixel(VkFormat _format)
{
	switch (_format) {
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8_SRGB:
		return 1; // 1 byte per pixel

	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8_SRGB:
		return 2; // 2 bytes per pixel

	case VK_FORMAT_R8G8B8_UNORM:
	case VK_FORMAT_R8G8B8_SNORM:
	case VK_FORMAT_R8G8B8_UINT:
	case VK_FORMAT_R8G8B8_SINT:
	case VK_FORMAT_R8G8B8_SRGB:
	case VK_FORMAT_B8G8R8_UNORM:
	case VK_FORMAT_B8G8R8_SNORM:
	case VK_FORMAT_B8G8R8_UINT:
	case VK_FORMAT_B8G8R8_SINT:
	case VK_FORMAT_B8G8R8_SRGB:
		return 3; // 3 bytes per pixel

	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SNORM:
	case VK_FORMAT_B8G8R8A8_UINT:
	case VK_FORMAT_B8G8R8A8_SINT:
	case VK_FORMAT_B8G8R8A8_SRGB:
		return 4; // 4 bytes per pixel

	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_SFLOAT:
		return 2; // 2 bytes per pixel

	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SFLOAT:
		return 4; // 4 bytes per pixel

	case VK_FORMAT_R16G16B16_UNORM:
	case VK_FORMAT_R16G16B16_SNORM:
	case VK_FORMAT_R16G16B16_UINT:
	case VK_FORMAT_R16G16B16_SINT:
	case VK_FORMAT_R16G16B16_SFLOAT:
		return 6; // 6 bytes per pixel

	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return 8; // 8 bytes per pixel

	case VK_FORMAT_R32_SFLOAT:
		return 4; // 4 bytes per pixel

	case VK_FORMAT_R32G32_SFLOAT:
		return 8; // 8 bytes per pixel

	case VK_FORMAT_R32G32B32_SFLOAT:
		return 12; // 12 bytes per pixel

	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return 16; // 16 bytes per pixel

	default:
		return -1; // Unsupported format
	}
}

TextureSystem::TextureSystem(VesperApp& _app, Device& _device)
	: m_app(_app)
	, m_device(_device)
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


std::shared_ptr<TextureData> TextureSystem::LoadCubemap(const std::array<std::string, 6>& _paths, VkFormat _overrideFormat)
{
	std::string hugePathNameForHash{""};
	for (const std::string& path : _paths)
	{
		hugePathNameForHash += path;
	}

	const uint32 hash = HashString(hugePathNameForHash.c_str());

	auto it = m_textureLookup.find(hash);
	if (it != m_textureLookup.end())
	{
		const int32 index = it->second;
		return m_textures[index];
	}

	auto texture = std::make_shared<TextureData>();

	uint32 totalFaceSize = 0;
	uint32 width = 0;
	uint32 height = 0;

	std::array<uint8*, 6> faceData;
	std::array<int32, 6> faceWidths, faceHeights, faceChannels;
	for (uint32 i = 0; i < 6; ++i)
	{
		faceData[i] = LoadTextureData(_paths[i], faceWidths[i], faceHeights[i], faceChannels[i], STBI_rgb_alpha);
		if (!faceData[i])
		{
			throw std::runtime_error("Failed to load texture for cubemap face: " + _paths[i]);
		}

		if (i == 0)
		{
			width = faceWidths[i];
			height = faceHeights[i];
			totalFaceSize = width * height * STBI_rgb_alpha; // Calculate size for one face
		}
		else if (width != faceWidths[i] || height != faceHeights[i])
		{
			throw std::runtime_error("Cubemap faces have mismatched dimensions");
		}
	}

	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // Default to SRGB 4 channels
	if (_overrideFormat == VK_FORMAT_UNDEFINED)
	{
		if (faceChannels[0] == 1)
		{
			format = VK_FORMAT_R8_UNORM; // Single channel (gray-scale)
		}
		else if (faceChannels[0] == 3)
		{
			format = VK_FORMAT_R8G8B8A8_SRGB; // Convert RGB to RGBA
		}
	}
	else
	{
		format = _overrideFormat;
	}

	// Allocate a single continuous buffer
	uint8* continuousBuffer = new uint8[totalFaceSize * 6];

	// Copy all face data into the continuous buffer
	for (uint32 i = 0; i < 6; ++i)
	{
		std::memcpy(continuousBuffer + i * totalFaceSize, faceData[i], totalFaceSize);
		FreeTextureData(faceData[i]);
		faceData[i] = continuousBuffer + i * totalFaceSize; // Optionally map pointers back to the single buffer
	}

	CreateTextureImage(continuousBuffer, width, height, format, texture->Image, texture->AllocationMemory, 6, 1, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

	texture->ImageView = CreateImageView(texture->Image, format, 6, 1);

	texture->Sampler = CreateTextureSampler();

	// free local buffer
	delete[] continuousBuffer;

	m_textures.push_back(texture);

	const int32 index = static_cast<int32>(m_textures.size() - 1);
	m_textureLookup[hash] = index;

	texture->Index = index;

	return texture;
}

std::shared_ptr<TextureData> TextureSystem::LoadCubemap(const std::string& _hdrPath, VkFormat _overrideFormat)
{
	const uint32 hash = HashString(_hdrPath.c_str());

	auto it = m_textureLookup.find(hash);
	if (it != m_textureLookup.end())
	{
		const int32 index = it->second;
		return m_textures[index];
	}

	auto texture = std::make_shared<TextureData>();

	// Load the HDR texture
	int32 width, height, channels;
	float* hdrData = stbi_loadf(_hdrPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!hdrData)
	{
		throw std::runtime_error("Failed to load HDR file: " + _hdrPath);
	}

	VkFormat format = (_overrideFormat != VK_FORMAT_UNDEFINED) ? _overrideFormat : VK_FORMAT_R32G32B32A32_SFLOAT;
	HDRProjectionType projection = HDRCubemapCPU::DetectHDRProjectionType(width, height);

	uint32 cubemapSize = 0;
	switch (projection)
	{
	case HDRProjectionType::Cube:
	{
		bool vertical = (width / 3 == height / 4);
		cubemapSize = vertical ? width / 3 : width / 4;
		break;
	}
	case HDRProjectionType::Hemisphere:
		cubemapSize = width / 2;
		break;
	case HDRProjectionType::Parabolic:
	case HDRProjectionType::LatLongCubemap:
	case HDRProjectionType::Equirectangular:
	default:
		cubemapSize = height / 2;
		break;
	}
	cubemapSize = std::clamp(cubemapSize, 64u, 2048u);

	std::vector<uint16_t> cubemap16;
	std::vector<float> cubemap32;

	if (format == VK_FORMAT_R16G16B16A16_SFLOAT)
	{
		cubemap16 = HDRCubemapCPU::GenerateFloat16Cubemap(hdrData, width, height, cubemapSize, projection);
		CreateTextureImage(cubemap16.data(), cubemapSize, cubemapSize, format, texture->Image, texture->AllocationMemory, 6, 1, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	}
	else
	{
		cubemap32 = HDRCubemapCPU::GenerateFloat32Cubemap(hdrData, width, height, cubemapSize, projection);
		CreateTextureImage(cubemap32.data(), cubemapSize, cubemapSize, format, texture->Image, texture->AllocationMemory, 6, 1, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	}

	texture->ImageView = CreateImageView(texture->Image, format, 6, 1);
	texture->Sampler = CreateTextureSampler();

	stbi_image_free(hdrData);

	m_textures.push_back(texture);

	const int32 index = static_cast<int32>(m_textures.size() - 1);
	m_textureLookup[hash] = index;

	texture->Index = index;

	return texture;
}

std::shared_ptr<TextureData> TextureSystem::GenerateBRDFLutTexture(const std::string& _name, VkExtent2D _extent)
{
	const uint32 hash = HashString(_name.c_str());

	auto it = m_textureLookup.find(hash);
	if (it != m_textureLookup.end())
	{
		const int32 index = it->second;
		return m_textures[index];
	}

	std::unique_ptr<OffscreenRenderer> offscreenRenderer = std::make_unique<OffscreenRenderer>(m_device, _extent, VK_FORMAT_R8G8B8A8_UNORM);
	std::unique_ptr<BRDFLUTGenerationSystem> m_brdfLutGenerationSystem = std::make_unique<BRDFLUTGenerationSystem>(m_app, m_device, offscreenRenderer->GetOffscreenSwapChainRenderPass());

	auto commandBuffer = offscreenRenderer->BeginFrame();
	offscreenRenderer->BeginOffscreenSwapChainRenderPass(commandBuffer);

	m_brdfLutGenerationSystem->Generate(commandBuffer, _extent.width, _extent.height);

	offscreenRenderer->EndOffscreenSwapChainRenderPass(commandBuffer);

	BufferComponent stagingBuffer = offscreenRenderer->PrepareImageCopy(commandBuffer);

	offscreenRenderer->EndFrame();

	std::vector<uint8> imageData = offscreenRenderer->FlushBufferToMemory(stagingBuffer);

	return LoadTexture(_name, VK_FORMAT_R8G8B8A8_UNORM, imageData.data(), _extent.width, _extent.height);
}

std::shared_ptr<TextureData> TextureSystem::GenerateIrradianceCubemap(const std::string& _name, uint32 _faceSize, std::shared_ptr<TextureData> _environment)
{
	const uint32 hash = HashString(_name.c_str());

	auto it = m_textureLookup.find(hash);
	if (it != m_textureLookup.end())
	{
		const int32 index = it->second;
		return m_textures[index];
	}

	VkExtent2D extent{ _faceSize, _faceSize };
	std::unique_ptr<OffscreenRenderer> offscreen = std::make_unique<OffscreenRenderer>(m_device, extent, VK_FORMAT_R8G8B8A8_UNORM);
	std::unique_ptr<IrradianceConvolutionGenerationSystem> system = std::make_unique<IrradianceConvolutionGenerationSystem>(m_app, m_device, offscreen->GetOffscreenSwapChainRenderPass());

	VkDescriptorImageInfo envInfo{};
	envInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	envInfo.imageView = _environment->ImageView;
	envInfo.sampler = _environment->Sampler;

	const glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::vec3 dirs[6]{ {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
	const glm::vec3 ups[6]{ {0,-1,0},{0,-1,0},{0,0,1},{0,0,-1},{0,-1,0},{0,-1,0} };

	const uint32 faceByteSize = _faceSize * _faceSize * 4;
	std::vector<uint8> cubemapBuffer(faceByteSize * 6);

	for (uint32 face = 0; face < 6; ++face)
	{
		auto cmd = offscreen->BeginFrame();
		offscreen->BeginOffscreenSwapChainRenderPass(cmd);

		glm::mat4 view = glm::lookAt(glm::vec3(0.0f), dirs[face], ups[face]);
		glm::mat4 vp = proj * view;
		system->Generate(cmd, envInfo, vp);

		offscreen->EndOffscreenSwapChainRenderPass(cmd);
		BufferComponent staging = offscreen->PrepareImageCopy(cmd);
		offscreen->EndFrame();

		m_buffer->Map(staging);
		std::memcpy(cubemapBuffer.data() + face * faceByteSize, staging.MappedMemory, faceByteSize);
		m_buffer->Unmap(staging);
		m_buffer->Destroy(staging);
	}

	auto texture = std::make_shared<TextureData>();
	CreateTextureImage(cubemapBuffer.data(), _faceSize, _faceSize, VK_FORMAT_R8G8B8A8_UNORM, texture->Image, texture->AllocationMemory, 6, 1, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

	texture->ImageView = CreateImageView(texture->Image, VK_FORMAT_R8G8B8A8_UNORM, 6, 1);
	texture->Sampler = CreateTextureSampler();

	m_textures.push_back(texture);

	const int32 index = static_cast<int32>(m_textures.size() - 1);
	m_textureLookup[hash] = index;

	texture->Index = index;

	return texture;
}

std::shared_ptr<TextureData> TextureSystem::GeneratePreFilteredEnvironmentMap(const std::string& _name, uint32 _faceSize, std::shared_ptr<TextureData> _environment)
{
	const uint32 hash = HashString(_name.c_str());
	auto it = m_textureLookup.find(hash);
	if (it != m_textureLookup.end())
	{
		const int32 index = it->second;
		return m_textures[index];
	}

	const uint32 mipLevels = static_cast<uint32>(std::floor(std::log2(_faceSize))) + 1;

	auto texture = std::make_shared<TextureData>();
	CreateTextureImage(nullptr, _faceSize, _faceSize, VK_FORMAT_R8G8B8A8_UNORM, texture->Image, texture->AllocationMemory, 6, mipLevels, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	texture->ImageView = CreateImageView(texture->Image, VK_FORMAT_R8G8B8A8_UNORM, 6, mipLevels);
	texture->Sampler = CreateTextureSampler();

	VkDescriptorImageInfo envInfo{};
	envInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	envInfo.imageView = _environment->ImageView;
	envInfo.sampler = _environment->Sampler;

	const glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::vec3 dirs[6]{ {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1} };
	const glm::vec3 ups[6]{ {0,-1,0},{0,-1,0},{0,0,1},{0,0,-1},{0,-1,0},{0,-1,0} };

	VkCommandBuffer cmd = m_device.BeginSingleTimeCommands();
	m_device.TransitionImageLayout(cmd, texture->Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 6, mipLevels);
	m_device.EndSingleTimeCommands(cmd);

	for (uint32 mip = 0; mip < mipLevels; ++mip)
	{
		const uint32 mipSize = std::max(1u, _faceSize >> mip);
		std::unique_ptr<OffscreenRenderer> offscreen = std::make_unique<OffscreenRenderer>(m_device, VkExtent2D{ mipSize, mipSize }, VK_FORMAT_R8G8B8A8_UNORM);
		std::unique_ptr<PreFilteredEnvironmentGenerationSystem> system = std::make_unique<PreFilteredEnvironmentGenerationSystem>(m_app, m_device, offscreen->GetOffscreenSwapChainRenderPass());

		std::vector<uint8> cubemapData(mipSize * mipSize * 4 * 6);

		float roughness = static_cast<float>(mip) / static_cast<float>(mipLevels - 1);

		for (uint32 face = 0; face < 6; ++face)
		{
			auto cb = offscreen->BeginFrame();
			offscreen->BeginOffscreenSwapChainRenderPass(cb);

			glm::mat4 view = glm::lookAt(glm::vec3(0.0f), dirs[face], ups[face]);
			glm::mat4 vp = proj * view;
			system->Generate(cb, envInfo, vp, roughness);

			offscreen->EndOffscreenSwapChainRenderPass(cb);
			BufferComponent staging = offscreen->PrepareImageCopy(cb);
			offscreen->EndFrame();

			m_buffer->Map(staging);
			std::memcpy(cubemapData.data() + face * mipSize * mipSize * 4, staging.MappedMemory, mipSize * mipSize * 4);
			m_buffer->Unmap(staging);
			m_buffer->Destroy(staging);
		}

		BufferComponent upload = m_buffer->Create<BufferComponent>(cubemapData.size(), 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
		m_buffer->Map(upload);
		m_buffer->WriteToBuffer(upload.MappedMemory, cubemapData.data(), cubemapData.size());
		m_buffer->Unmap(upload);

		VkCommandBuffer copyCmd = m_device.BeginSingleTimeCommands();
		m_device.CopyBufferToImage(copyCmd, upload.Buffer, texture->Image, mipSize, mipSize, 6, mip + 1);
		m_device.EndSingleTimeCommands(copyCmd);

		m_buffer->Destroy(upload);
	}

	cmd = m_device.BeginSingleTimeCommands();
	m_device.TransitionImageLayout(cmd, texture->Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 6, mipLevels);
	m_device.EndSingleTimeCommands(cmd);

	m_textures.push_back(texture);
	const int32 index = static_cast<int32>(m_textures.size() - 1);
	m_textureLookup[hash] = index;
	texture->Index = index;

	return texture;
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

VkImageView TextureSystem::CreateImageView(VkImage _image, VkFormat _format, uint32 _layerCount, uint32 _mipLevels)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = _image;
	viewInfo.viewType = [_layerCount]()
		{
			if (_layerCount == 6)
			{
				return VK_IMAGE_VIEW_TYPE_CUBE;
			}
			else
			{
				return VK_IMAGE_VIEW_TYPE_2D;
			}
		}();
	viewInfo.format = _format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = _mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = _layerCount;

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
