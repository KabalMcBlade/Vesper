// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\material_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/material_system.h"
#include "Systems/texture_system.h"
#include "Systems/uniform_buffer.h"

#include "Core/glm_config.h"

#include "Backend/device.h"
#include "Backend/buffer.h"

#include "Utility/hash.h"


VESPERENGINE_NAMESPACE_BEGIN


// For string paths
uint32 ComputeMaterialHash(
    const std::string& name,
    const std::vector<std::string>& texturePaths,
    const std::vector<std::any>& values,
    bool isTransparent,
    bool isDoubleSided,
    vesper::MaterialType type,
    const std::vector<int32>& uvIndices)
{
    size_t seed = HashString(name.c_str());

    for (const auto& path : texturePaths)
    {
        HashCombine(seed, path); // empty path included
    }

    for (const auto& val : values)
    {
        if (val.type() == typeid(float))
        {
            HashCombine(seed, std::any_cast<float>(val));
        }
        else if (val.type() == typeid(glm::vec4))
        {
            const auto& v = std::any_cast<glm::vec4>(val);
            HashCombine(seed, v.x); HashCombine(seed, v.y); HashCombine(seed, v.z); HashCombine(seed, v.w);
        }
    }

    HashCombine(seed, static_cast<int>(isTransparent));
    HashCombine(seed, static_cast<int>(isDoubleSided));
    HashCombine(seed, static_cast<int>(type));

    for (int32 uv : uvIndices)
    {
        HashCombine(seed, uv);
    }

    return static_cast<uint32>(seed);
}

// For loaded textures
uint32 ComputeMaterialHash(
    const std::string& name,
    const std::vector<std::shared_ptr<TextureData>>& textures,
    const std::vector<std::any>& values,
    bool isTransparent,
    bool isDoubleSided,
    MaterialType type,
    const std::vector<int32>& uvIndices)
{
    size_t seed = HashString(name.c_str());

    // Include texture pointers or indices
    for (const auto& tex : textures)
    {
        // You can use pointer address if textures are unique instances
        HashCombine(seed, reinterpret_cast<uintptr_t>(tex.get()));
        // Or, better, if they have a unique ID or path, hash that
        // if (tex) HashCombine(seed, tex->PathOrID);
    }

    // Include values � only the types you expect
    for (const auto& val : values)
    {
        if (val.type() == typeid(float))
        {
            HashCombine(seed, std::any_cast<float>(val));
        }
        else if (val.type() == typeid(glm::vec4))
        {
            const auto& v = std::any_cast<glm::vec4>(val);
            HashCombine(seed, v.x); HashCombine(seed, v.y); HashCombine(seed, v.z); HashCombine(seed, v.w);
        }
        // Extend for other types as needed
    }

    // Include flags and type
    HashCombine(seed, static_cast<int>(isTransparent));
    HashCombine(seed, static_cast<int>(isDoubleSided));
    HashCombine(seed, static_cast<int>(type));

    for (int32 uv : uvIndices)
    {
        HashCombine(seed, uv);
    }

    return static_cast<uint32>(seed);
}


const MaterialSystem::DefaultMaterialType MaterialSystem::DefaultPhongMaterial =
{
        "_DefaultPhongMaterial_",
        {
                "",
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
    false,
	MaterialType::Phong
};

MaterialSystem::MaterialSystem(Device& _device, TextureSystem& _textureSystem)
	: m_device(_device)
	, m_textureSystem(_textureSystem)
{
	m_buffer = std::make_unique<Buffer>(m_device);
}

std::shared_ptr<MaterialData> MaterialSystem::CreateMaterial(
	const std::string _name,
	const std::vector<std::string>& _texturePaths,
	const std::vector<std::any>& _values,
	bool _bIsTransparent,
    bool _bIsDoubleSided,
	MaterialType _type,
    const std::vector<int32>& _uvIndices)
{
    const uint32 hash = ComputeMaterialHash(_name, _texturePaths, _values, _bIsTransparent, _bIsDoubleSided, _type, _uvIndices);

	auto it = m_materialLookup.find(hash);
	if (it != m_materialLookup.end())
	{
		const int32 index = it->second;
		return m_materials[index];
	}

	auto material = std::make_shared<MaterialData>();

	material->Type = _type;
	material->IsTransparent = _bIsTransparent;
    material->IsDoubleSided = _bIsDoubleSided;

#ifdef _DEBUG
	material->Name = _name;
#endif

    switch (_type)
    {
    case vesper::MaterialType::PBR: 
    {
        const int32 texCount = 7;
        const int32 valCount = 9;
        material->Textures.resize(texCount);
        material->UVIndices.resize(texCount);
        assert(_texturePaths.size() == texCount &&"Texture array passed has not the amount of texture expected for material type!");
        assert(_values.size() == valCount && "Values array passed has not the amount of values expected for material type!");

        material->Textures[0] = _texturePaths[0].empty() ? m_textureSystem.LoadTexture(TextureSystem::RoughnessTexture) : m_textureSystem.LoadTexture(_texturePaths[0]);
        material->Textures[1] = _texturePaths[1].empty() ? m_textureSystem.LoadTexture(TextureSystem::MetallicTexture) : m_textureSystem.LoadTexture(_texturePaths[1]);
        material->Textures[2] = _texturePaths[2].empty() ? m_textureSystem.LoadTexture(TextureSystem::SheenTexture) : m_textureSystem.LoadTexture(_texturePaths[2]);
        material->Textures[3] = _texturePaths[3].empty() ? m_textureSystem.LoadTexture(TextureSystem::EmissiveTexture) : m_textureSystem.LoadTexture(_texturePaths[3]);
        material->Textures[4] = _texturePaths[4].empty() ? m_textureSystem.LoadTexture(TextureSystem::NormalTexture) : m_textureSystem.LoadTexture(_texturePaths[4]);
        material->Textures[5] = _texturePaths[5].empty() ? m_textureSystem.LoadTexture(TextureSystem::BaseColorTexture) : m_textureSystem.LoadTexture(_texturePaths[5]);
        material->Textures[6] = _texturePaths[6].empty() ? m_textureSystem.LoadTexture(TextureSystem::AOTexture) : m_textureSystem.LoadTexture(_texturePaths[6]);

        // const uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);
        material->UniformBuffer = m_buffer->Create<BufferComponent>(
            sizeof(PBRMaterialUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            /*minUboAlignment*/ 1, true);

        PBRMaterialUBO pbrUBO;
        if (m_device.IsBindlessResourcesSupported()) 
        {
            for (size_t i = 0; i < _texturePaths.size(); ++i) 
            {
                pbrUBO.TextureIndices[i] = _texturePaths[i].empty() ? -1 : m_textureSystem.GetTextureIndex(material->Textures[i]);
            }
        }
        else 
        {
            for (size_t i = 0; i < _texturePaths.size(); ++i) 
            {
                pbrUBO.TextureIndices[i] = _texturePaths[i].empty() ? -1 : 1;
            }
        }

        for (size_t i = 0; i < material->UVIndices.size(); ++i)
        {
            material->UVIndices[i] = (i < _uvIndices.size()) ? _uvIndices[i] : 0;
            pbrUBO.UVIndices[i] = material->UVIndices[i];
        }

        pbrUBO.Roughness = std::any_cast<float>(_values[0]);
        pbrUBO.Metallic = std::any_cast<float>(_values[1]);
        pbrUBO.Sheen = std::any_cast<float>(_values[2]);
        pbrUBO.ClearcoatThickness = std::any_cast<float>(_values[3]);
        pbrUBO.ClearcoatRoughness = std::any_cast<float>(_values[4]);
        pbrUBO.Anisotropy = std::any_cast<float>(_values[5]);
        pbrUBO.AnisotropyRotation = std::any_cast<float>(_values[6]);
        pbrUBO.AlphaCutoff = std::any_cast<float>(_values[7]);
        pbrUBO.BaseColorAlpha = std::any_cast<float>(_values[8]);

        material->UniformBuffer.MappedMemory = &pbrUBO;
        m_buffer->WriteToBuffer(material->UniformBuffer);

        break;
    }
    case vesper::MaterialType::Phong:
    default: 
    {
        const int32 texCount = 5;
        const int32 valCount = 5;
        material->Textures.resize(texCount);
        material->UVIndices.resize(texCount);
        assert(_texturePaths.size() == texCount && "Texture array passed has not the amount of texture expected for material type!");
        assert(_values.size() == valCount && "Values array passed has not the amount of values expected for material type!");

        material->Textures[0] = _texturePaths[0].empty() ? m_textureSystem.LoadTexture(TextureSystem::AmbientTexture) : m_textureSystem.LoadTexture(_texturePaths[0]);
        material->Textures[1] = _texturePaths[1].empty() ? m_textureSystem.LoadTexture(TextureSystem::DiffuseTexture) : m_textureSystem.LoadTexture(_texturePaths[1]);
        material->Textures[2] = _texturePaths[2].empty() ? m_textureSystem.LoadTexture(TextureSystem::SpecularTexture) : m_textureSystem.LoadTexture(_texturePaths[2]);
        material->Textures[3] = _texturePaths[3].empty() ? m_textureSystem.LoadTexture(TextureSystem::NormalTexture) : m_textureSystem.LoadTexture(_texturePaths[3]);
        material->Textures[4] = _texturePaths[4].empty() ? m_textureSystem.LoadTexture(TextureSystem::AlphaTexture) : m_textureSystem.LoadTexture(_texturePaths[4]);

        // const uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);
        material->UniformBuffer = m_buffer->Create<BufferComponent>(
            sizeof(PhongMaterialUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
            /*minUboAlignment*/ 1, true);

        PhongMaterialUBO phongUBO;
        if (m_device.IsBindlessResourcesSupported()) 
        {
            for (size_t i = 0; i < _texturePaths.size(); ++i) 
            {
                phongUBO.TextureIndices[i] = _texturePaths[i].empty() ? -1 : m_textureSystem.GetTextureIndex(material->Textures[i]);
            }
        }
        else 
        {
            for (size_t i = 0; i < _texturePaths.size(); ++i)
            {
                phongUBO.TextureIndices[i] = _texturePaths[i].empty() ? -1 : 1;
            }
        }

        for (size_t i = 0; i < material->UVIndices.size(); ++i)
        {
            material->UVIndices[i] = (i < _uvIndices.size()) ? _uvIndices[i] : 0;
            phongUBO.UVIndices[i] = material->UVIndices[i];
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

std::shared_ptr<MaterialData> MaterialSystem::CreateMaterial(
    const std::string _name,
    const std::vector<std::shared_ptr<TextureData>>& _textures,
    const std::vector<std::any>& _values,
    bool _bIsTransparent,
    bool _bIsDoubleSided,
    MaterialType _type,
    const std::vector<int32>& _uvIndices)
{
    const uint32 hash = ComputeMaterialHash(_name, _textures, _values, _bIsTransparent, _bIsDoubleSided, _type, _uvIndices);

    auto it = m_materialLookup.find(hash);
    if (it != m_materialLookup.end())
    {
        const int32 index = it->second;
        return m_materials[index];
    }

    auto material = std::make_shared<MaterialData>();

    material->Type = _type;
    material->IsTransparent = _bIsTransparent;
    material->IsDoubleSided = _bIsDoubleSided;

#ifdef _DEBUG
    material->Name = _name;
#endif

    switch (_type) 
    {
    case vesper::MaterialType::PBR: 
    {
        const int32 texCount = 7;
        const int32 valCount = 9;
        material->Textures.resize(texCount);
        material->UVIndices.resize(texCount);
        assert(_textures.size() == texCount && "Texture array passed has not the amount of texture expected for material type!");
        assert(_values.size() == valCount && "Values array passed has not the amount of values expected for material type!");

        material->Textures[0] = _textures[0] ? _textures[0] : m_textureSystem.LoadTexture(TextureSystem::RoughnessTexture);
        material->Textures[1] = _textures[1] ? _textures[1] : m_textureSystem.LoadTexture(TextureSystem::MetallicTexture);
        material->Textures[2] = _textures[2]  ? _textures[2] : m_textureSystem.LoadTexture(TextureSystem::SheenTexture);
        material->Textures[3] = _textures[3] ? _textures[3] : m_textureSystem.LoadTexture(TextureSystem::EmissiveTexture);
        material->Textures[4] = _textures[4] ? _textures[4] : m_textureSystem.LoadTexture(TextureSystem::NormalTexture);
        material->Textures[5] = _textures[5] ? _textures[5] : m_textureSystem.LoadTexture(TextureSystem::BaseColorTexture);
        material->Textures[6] = _textures[6]  ? _textures[6] : m_textureSystem.LoadTexture(TextureSystem::AOTexture);

        material->UniformBuffer = m_buffer->Create<BufferComponent>(
            sizeof(PBRMaterialUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT, 1, true);

        PBRMaterialUBO pbrUBO;
        if (m_device.IsBindlessResourcesSupported()) 
        {
            for (size_t i = 0; i < _textures.size(); ++i)
            {
                pbrUBO.TextureIndices[i] = _textures[i] ? m_textureSystem.GetTextureIndex(material->Textures[i]) : -1;
            }
        }
        else {
            for (size_t i = 0; i < _textures.size(); ++i)
            {
                pbrUBO.TextureIndices[i] = _textures[i] ? 1 : -1;
            }
        }

        for (size_t i = 0; i < material->UVIndices.size(); ++i)
        {
            material->UVIndices[i] = (i < _uvIndices.size()) ? _uvIndices[i] : 0;
            pbrUBO.UVIndices[i] = material->UVIndices[i];
        }

        pbrUBO.Roughness = std::any_cast<float>(_values[0]);
        pbrUBO.Metallic = std::any_cast<float>(_values[1]);
        pbrUBO.Sheen = std::any_cast<float>(_values[2]);
        pbrUBO.ClearcoatThickness = std::any_cast<float>(_values[3]);
        pbrUBO.ClearcoatRoughness = std::any_cast<float>(_values[4]);
        pbrUBO.Anisotropy = std::any_cast<float>(_values[5]);
        pbrUBO.AnisotropyRotation = std::any_cast<float>(_values[6]);
        pbrUBO.AlphaCutoff = std::any_cast<float>(_values[7]);
        pbrUBO.BaseColorAlpha = std::any_cast<float>(_values[8]);

        material->UniformBuffer.MappedMemory = &pbrUBO;
        m_buffer->WriteToBuffer(material->UniformBuffer);

        break;
    }
    case vesper::MaterialType::Phong:
    default: 
    {
        const int32 texCount = 5;
        const int32 valCount = 5;
        material->Textures.resize(texCount);
        material->UVIndices.resize(texCount);
        assert(_textures.size() == texCount && "Texture array passed has not the amount of texture expected for material type!");
        assert(_values.size() == valCount && "Values array passed has not the amount of values expected for material type!");

        material->Textures[0] = _textures[0] ? _textures[0] : m_textureSystem.LoadTexture(TextureSystem::AmbientTexture);
        material->Textures[1] = _textures[1] ? _textures[1] : m_textureSystem.LoadTexture(TextureSystem::DiffuseTexture);
        material->Textures[2] = _textures[2] ? _textures[2] : m_textureSystem.LoadTexture(TextureSystem::SpecularTexture);
        material->Textures[3] = _textures[3] ? _textures[3] : m_textureSystem.LoadTexture(TextureSystem::NormalTexture);
        material->Textures[4] = _textures[4] ? _textures[4] : m_textureSystem.LoadTexture(TextureSystem::AlphaTexture);

        material->UniformBuffer = m_buffer->Create<BufferComponent>(
            sizeof(PhongMaterialUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT, 1, true);

        PhongMaterialUBO phongUBO;
        if (m_device.IsBindlessResourcesSupported()) 
        {
            for (size_t i = 0; i < _textures.size(); ++i)
            {
                phongUBO.TextureIndices[i] = _textures[i] ? m_textureSystem.GetTextureIndex(material->Textures[i]) : -1;
            }
        }
        else
        {
            for (size_t i = 0; i < _textures.size(); ++i)
            {
                phongUBO.TextureIndices[i] = _textures[i] ? 1 : -1;
            }
        }

        for (size_t i = 0; i < material->UVIndices.size(); ++i)
        {
            material->UVIndices[i] = (i < _uvIndices.size()) ? _uvIndices[i] : 0;
            phongUBO.UVIndices[i] = material->UVIndices[i];
        }

        m_materials.push_back(material);
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
	return CreateMaterial(_defaultMaterial.Name, _defaultMaterial.Textures, _defaultMaterial.Values, _defaultMaterial.IsTransparent, _defaultMaterial.IsDoubleSided, _defaultMaterial.Type, {});
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
