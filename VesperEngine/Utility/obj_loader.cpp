#include "obj_loader.h"

#include "App/vesper_app.h"
#include "App/config.h"

#include "Systems/material_system.h"

#include "Utility/hash.h"
#include "Utility/logger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

// Maybe to move in a specific loader system for obj
#define TINYOBJLOADER_USE_MAPBOX_EARCUT	// robust triangulation
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
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

std::unique_ptr<MaterialData> CreateMaterialData(const tinyobj::material_t& _tinyMaterial, const Config& _config)
{
	// for now we support only Phong and PBR
	auto materialType = IsPBRMaterial(_tinyMaterial) ? MaterialType::PBR : MaterialType::Phong;

	if (materialType == MaterialType::Phong)
	{
		auto phongMaterial = std::make_unique<MaterialDataPhong>();

		phongMaterial->Type = MaterialType::Phong;
		phongMaterial->Name = _tinyMaterial.name;
		phongMaterial->Shininess = _tinyMaterial.shininess;

		phongMaterial->AmbientColor = glm::vec4(_tinyMaterial.ambient[0], _tinyMaterial.ambient[1], _tinyMaterial.ambient[2], _tinyMaterial.dissolve);
		phongMaterial->DiffuseColor = glm::vec4(_tinyMaterial.diffuse[0], _tinyMaterial.diffuse[1], _tinyMaterial.diffuse[2], _tinyMaterial.dissolve);
		phongMaterial->SpecularColor = glm::vec4(_tinyMaterial.specular[0], _tinyMaterial.specular[1], _tinyMaterial.specular[2], _tinyMaterial.dissolve);
		phongMaterial->EmissionColor = glm::vec4(_tinyMaterial.emission[0], _tinyMaterial.emission[1], _tinyMaterial.emission[2], _tinyMaterial.dissolve);

		phongMaterial->AmbientTexturePath = _config.TexturesPath + _tinyMaterial.ambient_texname;
		phongMaterial->DiffuseTexturePath = _config.TexturesPath + _tinyMaterial.diffuse_texname;
		phongMaterial->SpecularTexturePath = _config.TexturesPath + _tinyMaterial.specular_texname;
		phongMaterial->NormalTexturePath = _config.TexturesPath + _tinyMaterial.normal_texname;

		return phongMaterial;
	}
	else if (materialType == MaterialType::PBR)
	{
		auto pbrMaterial = std::make_unique<MaterialDataPBR>();

		pbrMaterial->Type = MaterialType::PBR;
		pbrMaterial->Name = _tinyMaterial.name;
		pbrMaterial->Roughness = _tinyMaterial.roughness;
		pbrMaterial->Metallic = _tinyMaterial.metallic;
		pbrMaterial->Sheen = _tinyMaterial.sheen;
		pbrMaterial->ClearcoatThickness = _tinyMaterial.clearcoat_thickness;
		pbrMaterial->ClearcoatRoughness = _tinyMaterial.clearcoat_roughness;
		pbrMaterial->Anisotropy = _tinyMaterial.anisotropy;
		pbrMaterial->AnisotropyRotation = _tinyMaterial.anisotropy_rotation;

		pbrMaterial->RoughnessTexturePath = _config.TexturesPath + _tinyMaterial.roughness_texname;
		pbrMaterial->MetallicTexturePath = _config.TexturesPath + _tinyMaterial.metallic_texname;
		pbrMaterial->SheenTexturePath = _config.TexturesPath + _tinyMaterial.sheen_texname;
		pbrMaterial->EmissiveTexturePath = _config.TexturesPath + _tinyMaterial.emissive_texname;
		pbrMaterial->NormalMapTexturePath = _config.TexturesPath + _tinyMaterial.normal_texname;

		return pbrMaterial;
	}
	else
	{
		// Fallback to a default MaterialData if material type is invalid
		auto defaultMaterial = MaterialSystem::CreateDefaultMaterialData();

		return defaultMaterial;
	}
}


ObjLoader::ObjLoader(VesperApp& _app, Device& _device)
	: m_app(_app)
	, m_device{_device}
{

}

std::vector<std::unique_ptr<ModelData>> ObjLoader::LoadModel(const std::string& _fileName, bool _isStatic)
{
	std::vector<std::unique_ptr<ModelData>> models;

	tinyobj::ObjReaderConfig readerConfig;
	readerConfig.mtl_search_path = m_app.GetConfig().ModelsPath;

	tinyobj::ObjReader reader;
	if (!reader.ParseFromFile(m_app.GetConfig().ModelsPath + _fileName, readerConfig))
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
		auto model = std::make_unique<ModelData>();
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		const auto& material_ids = shape.mesh.material_ids;

		for (size_t faceIndex = 0; faceIndex < shape.mesh.indices.size() / 3; ++faceIndex)
		{
			if (hasMaterials)
			{
				const int32 materialID = material_ids[faceIndex];

				if (materialID >= 0 && materialID < static_cast<int32>(materials.size()))
				{
					const auto& tinyMaterial = materials[materialID];
					model->Material = CreateMaterialData(tinyMaterial, m_app.GetConfig());
				}
				else
				{
					LOG(Logger::ERROR, "Material ID out of range for face: ", faceIndex);
				}
			}
			else
			{
				// No materials available; assign a default
				model->Material = MaterialSystem::CreateDefaultMaterialData();
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

					vertex.Color =
					{
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2],
					};
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
					vertex.UV =
					{
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1],
					};
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(model->Vertices.size());
					model->Vertices.push_back(vertex);
				}

				model->Indices.push_back(uniqueVertices[vertex]);
			}
		}

		model->IsStatic = _isStatic;

		LOG(Logger::INFO, "Shape: ", shape.name);
		LOG(Logger::INFO, "Material: ", model->Material->Name);
		LOG(Logger::INFO, "Vertices count: ", model->Vertices.size());
		LOG(Logger::INFO, "Indices count: ", model->Indices.size());
		LOG_NL();

		// Add the model for this shape to the list
		models.push_back(std::move(model));
	}

	LOG(Logger::INFO, "Total shapes processed: ", models.size());
	LOG_NL();
	LOG_NL();

	return models;
}

VESPERENGINE_NAMESPACE_END
