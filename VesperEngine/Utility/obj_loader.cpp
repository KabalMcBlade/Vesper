#include "obj_loader.h"

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

ObjLoader::ObjLoader(Device& _device)
	: m_device{_device}
{

}

std::vector<std::unique_ptr<ModelData>> ObjLoader::LoadModel(const std::string& _filePath, bool _isStatic)
{
	std::vector<std::unique_ptr<ModelData>> models;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning;
	std::string error;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, _filePath.c_str()))
	{
		throw std::runtime_error(warning + error);
	}

	for (const auto& shape : shapes)
	{
		// Create a new ModelData for this shape
		auto model = std::make_unique<ModelData>();
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& index : shape.mesh.indices)
		{
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

		model->IsStatic = _isStatic;

		LOG(Logger::INFO, "Shape: ", shape.name);
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
