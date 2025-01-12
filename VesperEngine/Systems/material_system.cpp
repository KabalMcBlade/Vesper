// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\material_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "material_system.h"

#include "Backend/offscreen_renderer.h"

#include "App/vesper_app.h"
#include "App/file_system.h"

#include "Systems/brdf_lut_generation_system.h"

#include "Utility/hash.h"
#include "Utility/logger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"


#include "../stb/stb_image.h"

#include <random>


VESPERENGINE_NAMESPACE_BEGIN

struct VESPERENGINE_ALIGN16 PhongMaterialUBO
{
	glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 DiffuseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 SpecularColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	glm::vec4 EmissionColor{ 0.0f, 0.0f, 0.0f, 1.0f };

	float Shininess{ 0.5f };

	int32 TextureFlags{ 0 };      // 0: HasAmbientTexture, 1: HasDiffuseTexture, 2: HasSpecularTexture, 3: HasNormalTexture
};

struct VESPERENGINE_ALIGN16 PBRMaterialUBO
{
	float Roughness{ 0.0f };
	float Metallic{ 0.0f };
	float Sheen{ 0.0f };
	float ClearcoatThickness{ 0.0f };
	float ClearcoatRoughness{ 0.0f };
	float Anisotropy{ 0.0f };
	float AnisotropyRotation{ 0.0f };

	int32 TextureFlags{ 0 };      // 0: HasRoughnessTexture, 1: HasMetallicTexture, 2: HasSheenTexture, 3: HasEmissiveTexture, 4: HasNormalTexture
};


MaterialSystem::MaterialSystem(Device& _device, TextureSystem& _textureSystem)
	: m_device(_device)
	, m_textureSystem(_textureSystem)
{
	m_buffer = std::make_unique<Buffer>(m_device);
}

const MaterialSystem::DefaultMaterialType MaterialSystem::DefaultPhongMaterial =
{
	"_DefaultPhongMaterial_",
	{
		"",
		"",
		"",
		"",
	},
	{ 
		glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
		glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
		glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
		glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
		float{0.5f}
	},
	false,
	MaterialType::Phong
};

std::shared_ptr<MaterialData> MaterialSystem::CreateMaterial(
	const std::string _name,
	const std::vector<std::string>& _texturePaths,
	const std::vector<std::any>& _values,
	bool _bIsTransparent,
	MaterialType _type)
{
	const uint32 hash = HashString(_name.c_str());

	auto it = m_materialLookup.find(hash);
	if (it != m_materialLookup.end())
	{
		const int32 index = it->second;
		return m_materials[index];
	}

	auto material = std::make_shared<MaterialData>();

	material->Type = _type;
	material->IsTransparent = _bIsTransparent;

#ifdef _DEBUG
	material->Name = _name;
#endif

	switch (_type)
	{
	case vesper::MaterialType::PBR:
	{
		const int32 texCount = 5;
		const int32 valCount = 7;
		material->Textures.resize(texCount);
		assert(_texturePaths.size() == texCount && "Texture array passed has not the amount of texture expected for material type!");
		assert(_values.size() == valCount && "Values array passed has not the amount of values expected for material type!");

		material->Textures[0] = _texturePaths[0].empty() ? m_textureSystem.LoadTexture(TextureSystem::RoughnessTexture) : m_textureSystem.LoadTexture(_texturePaths[0]);
		material->Textures[1] = _texturePaths[1].empty() ? m_textureSystem.LoadTexture(TextureSystem::MetallicTexture) : m_textureSystem.LoadTexture(_texturePaths[1]);
		material->Textures[2] = _texturePaths[2].empty() ? m_textureSystem.LoadTexture(TextureSystem::SheenTexture) : m_textureSystem.LoadTexture(_texturePaths[2]);
		material->Textures[3] = _texturePaths[3].empty() ? m_textureSystem.LoadTexture(TextureSystem::EmissiveTexture) : m_textureSystem.LoadTexture(_texturePaths[3]);
		material->Textures[4] = _texturePaths[4].empty() ? m_textureSystem.LoadTexture(TextureSystem::NormalTexture) : m_textureSystem.LoadTexture(_texturePaths[4]);

		const uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);
		material->UniformBuffer = m_buffer->Create<BufferComponent>(
			sizeof(PBRMaterialUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			minUboAlignment,
			true
		);

		PBRMaterialUBO pbrUBO;
		pbrUBO.TextureFlags = 0;	// 0: HasRoughnessTexture, 1: HasMetallicTexture, 2: HasSheenTexture, 3: HasEmissiveTexture, 4: HasNormalTexture

		for (size_t i = 0; i < _texturePaths.size(); ++i)
		{
			const int32 v = !_texturePaths[i].empty() ? 1 : 0;	// Set v to 1 if the string is not empty, 0 otherwise
			pbrUBO.TextureFlags |= (v << i);					// Perform the bitwise operation
		}

		pbrUBO.Roughness = std::any_cast<float>(_values[0]);
		pbrUBO.Metallic = std::any_cast<float>(_values[1]);
		pbrUBO.Sheen = std::any_cast<float>(_values[2]);
		pbrUBO.ClearcoatThickness = std::any_cast<float>(_values[3]);
		pbrUBO.ClearcoatRoughness = std::any_cast<float>(_values[4]);
		pbrUBO.Anisotropy = std::any_cast<float>(_values[5]);
		pbrUBO.AnisotropyRotation = std::any_cast<float>(_values[6]);

		material->UniformBuffer.MappedMemory = &pbrUBO;
		m_buffer->WriteToBuffer(material->UniformBuffer);

		break;
	}
	case vesper::MaterialType::Phong:
	default:
	{
		const int32 texCount = 4;
		const int32 valCount = 5;
		material->Textures.resize(texCount);
		assert(_texturePaths.size() == texCount && "Texture array passed has not the amount of texture expected for material type!");
		assert(_values.size() == valCount && "Values array passed has not the amount of values expected for material type!");

		material->Textures[0] = _texturePaths[0].empty() ? m_textureSystem.LoadTexture(TextureSystem::AmbientTexture) : m_textureSystem.LoadTexture(_texturePaths[0]);
		material->Textures[1] = _texturePaths[1].empty() ? m_textureSystem.LoadTexture(TextureSystem::DiffuseTexture) : m_textureSystem.LoadTexture(_texturePaths[1]);
		material->Textures[2] = _texturePaths[2].empty() ? m_textureSystem.LoadTexture(TextureSystem::SpecularTexture) : m_textureSystem.LoadTexture(_texturePaths[2]);
		material->Textures[3] = _texturePaths[3].empty() ? m_textureSystem.LoadTexture(TextureSystem::NormalTexture) : m_textureSystem.LoadTexture(_texturePaths[3]);

		const uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);
		material->UniformBuffer = m_buffer->Create<BufferComponent>(
			sizeof(PhongMaterialUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			minUboAlignment,
			true
		);

		PhongMaterialUBO phongUBO;
		phongUBO.TextureFlags = 0;	// 0: HasAmbientTexture, 1: HasDiffuseTexture, 2: HasSpecularTexture, 3: HasNormalTexture

		for (size_t i = 0; i < _texturePaths.size(); ++i)
		{
			const int32 v = !_texturePaths[i].empty() ? 1 : 0;	// Set v to 1 if the string is not empty, 0 otherwise
			phongUBO.TextureFlags |= (v << i);					// Perform the bitwise operation
		}

		phongUBO.AmbientColor = std::any_cast<glm::vec4>(_values[0]);
		phongUBO.DiffuseColor = std::any_cast<glm::vec4>(_values[1]);
		phongUBO.SpecularColor = std::any_cast<glm::vec4>(_values[2]);
		phongUBO.EmissionColor = std::any_cast<glm::vec4>(_values[3]);
		phongUBO.Shininess = std::any_cast<float>(_values[4]);

		material->UniformBuffer.MappedMemory = &phongUBO;
		m_buffer->WriteToBuffer(material->UniformBuffer);

		break;
	}
	}

	m_materials.push_back(material);

	const int32 index = static_cast<int32>(m_materials.size() - 1);
	m_materialLookup[hash] = index;

	material->Index = index;

	return material;
}

std::shared_ptr<MaterialData> MaterialSystem::CreateMaterial(const DefaultMaterialType& _defaultMaterial)
{
	return CreateMaterial(_defaultMaterial.Name, _defaultMaterial.Textures, _defaultMaterial.Values, _defaultMaterial.IsTransparent, _defaultMaterial.Type);
}

void MaterialSystem::Cleanup()
{
	for (const auto& material : m_materials)
	{
		m_buffer->Destroy(material->UniformBuffer);
	}

	m_materials.clear();
	m_materialLookup.clear();
}

VESPERENGINE_NAMESPACE_END
