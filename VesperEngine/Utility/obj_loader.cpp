#include "obj_loader.h"

#include "App/vesper_app.h"

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
			// still creating an empty material here
			auto material = std::make_unique<MaterialData>();

			if (hasMaterials)
			{
				const int32 materialID = material_ids[faceIndex];

				if (materialID >= 0 && materialID < static_cast<int32>(materials.size()))
				{
					const auto& tinyMaterial = materials[materialID];

					material->Type = IsPBRMaterial(tinyMaterial) ? MaterialType::PBR : MaterialType::Phong;
					material->Name = tinyMaterial.name;
					material->bIsBound = false;
			
					if (material->Type == MaterialType::Phong)
					{
						auto* phongData = dynamic_cast<MaterialDataPhong*>(material.get());

						phongData->Shininess = tinyMaterial.shininess;

						phongData->AmbientColor = glm::vec4(tinyMaterial.ambient[0], tinyMaterial.ambient[1], tinyMaterial.ambient[2], tinyMaterial.dissolve);
						phongData->DiffuseColor = glm::vec4(tinyMaterial.diffuse[0], tinyMaterial.diffuse[1], tinyMaterial.diffuse[2], tinyMaterial.dissolve);
						phongData->SpecularColor = glm::vec4(tinyMaterial.specular[0], tinyMaterial.specular[1], tinyMaterial.specular[2], tinyMaterial.dissolve);
						phongData->EmissionColor = glm::vec4(tinyMaterial.emission[0], tinyMaterial.emission[1], tinyMaterial.emission[2], tinyMaterial.dissolve);

						phongData->AmbientTexturePath = m_app.GetConfig().TexturesPath + tinyMaterial.ambient_texname;
						phongData->DiffuseTexturePath = m_app.GetConfig().TexturesPath + tinyMaterial.diffuse_texname;
						phongData->SpecularTexturePath = m_app.GetConfig().TexturesPath + tinyMaterial.specular_texname;
						phongData->NormalTexturePath = m_app.GetConfig().TexturesPath + tinyMaterial.normal_texname;
					}
					else if (material->Type == MaterialType::PBR)
					{
						auto* pbrData = dynamic_cast<MaterialDataPBR*>(material.get());

						pbrData->Roughness = tinyMaterial.roughness;	// can be derived by: 1.0f - tinyMaterial.specular[0];
						pbrData->Metallic = tinyMaterial.metallic;		// can be derived by: (tinyMaterial.illum == 2) ? 1.0f : 0.0f;
						pbrData->Sheen = tinyMaterial.sheen;
						pbrData->ClearcoatThickness = tinyMaterial.clearcoat_thickness;
						pbrData->ClearcoatRoughness = tinyMaterial.clearcoat_roughness;
						pbrData->Anisotropy = tinyMaterial.anisotropy;
						pbrData->AnisotropyRotation = tinyMaterial.anisotropy_rotation;

						pbrData->RoughnessTexturePath = m_app.GetConfig().TexturesPath + tinyMaterial.roughness_texname;
						pbrData->MetallicTexturePath = m_app.GetConfig().TexturesPath + tinyMaterial.metallic_texname;
						pbrData->SheenTexturePath = m_app.GetConfig().TexturesPath + tinyMaterial.sheen_texname;
						pbrData->EmissiveTexturePath = m_app.GetConfig().TexturesPath + tinyMaterial.emissive_texname;
						pbrData->NormalMapTexturePath = m_app.GetConfig().TexturesPath + tinyMaterial.normal_texname;
					}
					else
					{
						LOG(Logger::ERROR, "Material exist by is not a valid type! (Type: %d)", static_cast<uint32>(material->Type));
					}
				}
			}

			// Assign material to the model
			model->Material = std::move(material);
			
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
