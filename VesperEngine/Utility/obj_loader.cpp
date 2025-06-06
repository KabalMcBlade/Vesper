// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\obj_loader.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Utility/obj_loader.h"
#include "Utility/hash.h"
#include "Utility/logger.h"

#include "Core/glm_config.h"

#include "App/vesper_app.h"
#include "App/config.h"
#include "App/file_system.h"

#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/model_data.h"

#include "Systems/material_system.h"

// Maybe to move in a specific loader system for obj
#define TINYOBJLOADER_USE_MAPBOX_EARCUT	// robust triangulation
#define TINYOBJLOADER_IMPLEMENTATION
#include "ThirdParty/TinyObjLoader/tiny_obj_loader.h"

#include <stdexcept>
#include <unordered_map>


namespace std
{
	template <>
	struct hash<vesper::Vertex>
	{
		size_t operator()(vesper::Vertex const& _vertex) const
		{
			size_t seed = 0;
			vesper::HashCombine(seed, _vertex.Position, _vertex.Color, _vertex.Normal, _vertex.UV);
			return seed;
		}
	};
}  // namespace std



VESPERENGINE_NAMESPACE_BEGIN

bool IsPBRMaterial(const tinyobj::material_t& _material)
{
	constexpr float epsilon = 1e-6f;

	return (_material.metallic > epsilon ||
		_material.roughness > epsilon ||
		_material.sheen > epsilon ||
		_material.clearcoat_thickness > epsilon ||
		_material.clearcoat_roughness > epsilon ||
		_material.anisotropy > epsilon ||
		_material.anisotropy_rotation > epsilon);
}

std::shared_ptr<MaterialData> CreateMaterial(MaterialSystem& _materialSystem, const tinyobj::material_t& _tinyMaterial, const std::string& _texturePath)
{
	// for now we support only Phong and PBR
	auto materialType = IsPBRMaterial(_tinyMaterial) ? MaterialType::PBR : MaterialType::Phong;

	if (materialType == MaterialType::Phong)
	{
		return _materialSystem.CreateMaterial(
			_tinyMaterial.name,
			{
				_tinyMaterial.ambient_texname.empty() ? "" : _texturePath + _tinyMaterial.ambient_texname,
				_tinyMaterial.diffuse_texname.empty() ? "" : _texturePath + _tinyMaterial.diffuse_texname,
				_tinyMaterial.specular_texname.empty() ? "" : _texturePath + _tinyMaterial.specular_texname,
				_tinyMaterial.normal_texname.empty() ? "" : _texturePath + _tinyMaterial.normal_texname,
                _tinyMaterial.alpha_texname.empty() ? "" : _texturePath + _tinyMaterial.alpha_texname,
			},
			{
				glm::vec4(_tinyMaterial.ambient[0], _tinyMaterial.ambient[1], _tinyMaterial.ambient[2], 1.0f),
				glm::vec4(_tinyMaterial.diffuse[0], _tinyMaterial.diffuse[1], _tinyMaterial.diffuse[2], _tinyMaterial.dissolve),
				glm::vec4(_tinyMaterial.specular[0],_tinyMaterial.specular[1],_tinyMaterial.specular[2],_tinyMaterial.dissolve),
				glm::vec4(_tinyMaterial.emission[0], _tinyMaterial.emission[1], _tinyMaterial.emission[2], 1.0f),
				_tinyMaterial.shininess
			},
			_tinyMaterial.dissolve < 1.0f || !_tinyMaterial.alpha_texname.empty(),
			MaterialType::Phong
		);
	}
	else if (materialType == MaterialType::PBR)
	{
		return _materialSystem.CreateMaterial(
			_tinyMaterial.name,
			{
				_tinyMaterial.roughness_texname.empty() ? "" : _texturePath + _tinyMaterial.roughness_texname,
				_tinyMaterial.metallic_texname.empty() ? "" : _texturePath + _tinyMaterial.metallic_texname,
				_tinyMaterial.sheen_texname.empty() ? "" : _texturePath + _tinyMaterial.sheen_texname,
				_tinyMaterial.emissive_texname.empty() ? "" : _texturePath + _tinyMaterial.emissive_texname,
				_tinyMaterial.normal_texname.empty() ? "" : _texturePath + _tinyMaterial.normal_texname
			},
			{
				_tinyMaterial.roughness,
				_tinyMaterial.metallic,
				_tinyMaterial.sheen,
				_tinyMaterial.clearcoat_thickness,
				_tinyMaterial.clearcoat_roughness,
				_tinyMaterial.anisotropy,
				_tinyMaterial.anisotropy_rotation,
			},
			_tinyMaterial.dissolve < 1.0f,
			MaterialType::PBR
		);
	}
	else
	{
		// Fallback to a default MaterialData if material type is invalid
		return _materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
	}
}

ObjLoader::ObjLoader(VesperApp& _app, Device& _device, MaterialSystem& _materialSystem)
	: m_app(_app)
	, m_device{_device}
	, m_materialSystem{_materialSystem}
{
}

std::vector<std::unique_ptr<ModelData>> ObjLoader::LoadModel(const std::string& _fileName, bool _isStatic)
{
	const bool bIsFilepath = FileSystem::IsFilePath(_fileName);

	const std::string modelPath = bIsFilepath ? FileSystem::GetDirectoryPath(_fileName) : m_app.GetConfig().ModelsPath;
	const std::string texturePath = bIsFilepath ? FileSystem::GetDirectoryPath(_fileName, true) + m_app.GetConfig().TexturesFolderName : m_app.GetConfig().TexturesPath;
	const std::string actualFilename = bIsFilepath ? FileSystem::GetFileName(_fileName) : _fileName;

	std::vector<std::unique_ptr<ModelData>> models;

	tinyobj::ObjReaderConfig readerConfig;
	readerConfig.mtl_search_path = modelPath;

	tinyobj::ObjReader reader;
	if (!reader.ParseFromFile(modelPath + actualFilename, readerConfig))
	{
		throw std::runtime_error(reader.Warning() + reader.Error());
	}

	if (!reader.Warning().empty())
	{
		LOG(Logger::WARNING, "TinyObjReader: ", reader.Warning());
	}

	const auto& attrib = reader.GetAttrib();
	const auto& shapes = reader.GetShapes();
	const auto& materials = reader.GetMaterials();

	bool hasMaterials = !materials.empty();

	if (!hasMaterials)
	{
		LOG(Logger::WARNING, "Failed to load material file(s).");
	}

	for (const auto& shape : shapes)
	{
		// Map material id -> model data and vertex dedup map
		std::unordered_map<int32, std::unique_ptr<ModelData>> modelsPerMaterial;
		std::unordered_map<int32, std::unordered_map<Vertex, uint32>> uniqueVerticesPerMaterial;

		const auto& material_ids = shape.mesh.material_ids;

		for (size_t faceIndex = 0; faceIndex < shape.mesh.indices.size() / 3; ++faceIndex)
		{
			int32 materialID = -1;
			if (hasMaterials && !material_ids.empty())
			{
				materialID = material_ids[faceIndex];
			}

			auto& model = modelsPerMaterial[materialID];
			auto& uniqueVertices = uniqueVerticesPerMaterial[materialID];

			if (!model)
			{
				model = std::make_unique<ModelData>();

				if (hasMaterials && materialID >= 0 && materialID < static_cast<int32>(materials.size()))
				{
					const auto& tinyMaterial = materials[materialID];
					model->Material = CreateMaterial(m_materialSystem, tinyMaterial, texturePath);
				}
				else
				{
					if (materialID >= static_cast<int32>(materials.size()))
					{
						LOG(Logger::ERROR, "Material ID out of range for shape: ", shape.name);
					}
					model->Material = m_materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
				}
			}

			for (int32 vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
			{
				const auto& index = shape.mesh.indices[faceIndex * 3 + vertexIndex];
				Vertex vertex{};

				if (index.vertex_index >= 0)
				{
					vertex.Position =
					{
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
					};

					if (attrib.colors.size() > 0) 
					{
						vertex.Color = 
						{
							attrib.colors[3 * index.vertex_index + 0],
							attrib.colors[3 * index.vertex_index + 1],
							attrib.colors[3 * index.vertex_index + 2],
						};
					}
					else 
					{
						vertex.Color = { 1.0f, 1.0f, 1.0f };
					}
				}

				if (index.normal_index >= 0)
				{
					vertex.Normal =
					{
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2],
					};
				}

				if (index.texcoord_index >= 0)
				{
					/**
					 * OBJ File Format: Texture coordinates in OBJ files are typically in the range [0, 1], 
					 * but their origin is in the bottom-left corner of the texture.
					 * Vulkan: The origin of Vulkan’s texture coordinate system is in the top-left corner of the texture. 
					 * This discrepancy causes textures to appear flipped vertically when mapped onto your geometry.
					 */
					vertex.UV =
					{
// 						attrib.texcoords[2 * index.texcoord_index + 0],
// 						attrib.texcoords[2 * index.texcoord_index + 1],
						attrib.texcoords[2 * index.texcoord_index + 0],			// U-coordinate (unchanged)
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]	// Invert V-coordinate
					};

					// Normalize UV coordinates to [0, 1]
					vertex.UV.x = fmod(vertex.UV.x, 1.0f);
					vertex.UV.y = fmod(vertex.UV.y, 1.0f);

					// Ensure UVs are positive
					if (vertex.UV.x < 0.0f) 
					{
						vertex.UV.x += 1.0f;
					}
					if (vertex.UV.y < 0.0f)
					{
						vertex.UV.y += 1.0f;
					}
				}
				else 
				{
					vertex.UV = { 0.0f, 0.0f }; 
				}


				auto it = uniqueVertices.find(vertex);
				if (it == uniqueVertices.end())
				{
					uniqueVertices[vertex] = static_cast<uint32>(model->Vertices.size());
					model->Vertices.push_back(vertex);
				}

				model->Indices.push_back(uniqueVertices[vertex]);
			}
		}

		for (auto& [matID, model] : modelsPerMaterial)
		{
			model->IsStatic = _isStatic;

			LOG(Logger::INFO, "Shape: ", shape.name);
			LOG(Logger::INFO, "Material: ", model->Material->Name);
			LOG(Logger::INFO, "Vertices count: ", model->Vertices.size());
			LOG(Logger::INFO, "Indices count: ", model->Indices.size());
			LOG_NL();

			models.push_back(std::move(model));
		}
	}

	LOG(Logger::INFO, "Total shapes processed: ", models.size());
	LOG_NL();
	LOG_NL();

	return models;
}

VESPERENGINE_NAMESPACE_END
