// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\material_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Backend/model_data.h"

#include "vulkan/vulkan.h"

#include <any>
#include <string>
#include <vector>
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

class Buffer;
class TextureSystem;
class Device;

struct MaterialData;

/**
 * 2D Sampler list 
 * 
 * Phong Textures order:
 * binding = 0: uniform sampler2D ambientTexture;
 * binding = 1: uniform sampler2D diffuseTexture;
 * binding = 2: uniform sampler2D specularTexture;
 * binding = 3: uniform sampler2D normalTexture;
 * binding = 4: uniform sampler2D alphaTexture;
 * 
 * PBR Textures order:
 * binding = 0: uniform sampler2D roughnessTexture;
 * binding = 1: uniform sampler2D metallicTexture;
 * binding = 2: uniform sampler2D sheenTexture;
 * binding = 3: uniform sampler2D emissiveTexture;
 * binding = 4: uniform sampler2D normalTexture;
 * binding = 5: uniform sampler2D baseColorTexture;
 * binding = 6: uniform sampler2D aoTexture;
 */


 /**
  * Uniform buffers list
  * 
  * Phong uniform values:
  * glm::vec4 AmbientColor
  * glm::vec4 DiffuseColor
  * glm::vec4 SpecularColor
  * glm::vec4 EmissionColor
  * float Shininess
  * 
  * PBR uniform values
  * float Roughness
  * float Metallic
  * float Sheen
  * float ClearcoatThickness
  * float ClearcoatRoughness
  * float Anisotropy
  * float AnisotropyRotation
  * 
  */

class VESPERENGINE_API MaterialSystem final
{
private:
	struct DefaultMaterialType
	{
		std::string Name;
		const std::vector<std::string>& Textures;
		const std::vector<std::any> Values;
		bool IsTransparent;
		bool IsDoubleSided;
		MaterialType Type;
	};

public:
	static const DefaultMaterialType DefaultPhongMaterial;

public:
	MaterialSystem(Device& _device, TextureSystem& _textureSystem);
	~MaterialSystem() = default;

	MaterialSystem(const MaterialSystem&) = delete;
	MaterialSystem& operator=(const MaterialSystem&) = delete;

public:
	// the textures array MUST follow the order mentioned in the comment at the top!
	// leave empty "" to use default texture
	std::shared_ptr<MaterialData>  CreateMaterial(
		const std::string _name,
		const std::vector<std::string>& _texturePaths,
		const std::vector<std::any>& _values,
		bool _bIsTransparent,
		bool _bIsDoubleSided,
		MaterialType _type,
		const std::vector<int32>& _uvIndices = {});

	// Create material using textures already loaded. Pass nullptr entries to
	// use default textures. The textures array MUST follow the order
	// mentioned in the comment at the top of this file.
	std::shared_ptr<MaterialData>  CreateMaterial(
		const std::string _name,
		const std::vector<std::shared_ptr<TextureData>>& _textures,
		const std::vector<std::any>& _values,
		bool _bIsTransparent,
		bool _bIsDoubleSided,
		MaterialType _type,
		const std::vector<int32>& _uvIndices = {});

	std::shared_ptr<MaterialData> CreateMaterial(const DefaultMaterialType& _defaultMaterial);

	VESPERENGINE_INLINE std::shared_ptr<MaterialData> GetMaterial(uint32 _index) const
	{
		assert(_index >= 0 && _index < m_materials.size() && "Materials index is out of bound!");
		return m_materials[_index];
	}

	VESPERENGINE_INLINE const std::vector<std::shared_ptr<MaterialData>>& GetMaterials() const
	{
		return m_materials;
	}

	VESPERENGINE_INLINE TextureSystem& GetTextureSystem()
	{
		return m_textureSystem;
	}

	VESPERENGINE_INLINE const TextureSystem& GetTextureSystem() const
	{
		return m_textureSystem;
	}

	void Cleanup();

private:
	Device& m_device;
	TextureSystem& m_textureSystem;

	std::unique_ptr<Buffer> m_buffer;

	std::vector<std::shared_ptr<MaterialData>> m_materials;
	std::unordered_map<uint32, int32> m_materialLookup;
};

VESPERENGINE_NAMESPACE_END
