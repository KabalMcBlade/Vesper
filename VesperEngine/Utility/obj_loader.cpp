#include "pch.h"
#include "obj_loader.h"

#include "Utility/hash.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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

std::unique_ptr<ModelData> ObjLoader::LoadModel(const std::string& _filePath, bool _isStatic)
{
	ModelData model;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning;
	std::string error;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, _filePath.c_str()))
	{
		throw std::runtime_error(warning + error);
	}

	std::unordered_map<Vertex, uint32> uniqueVertices{};
	for (const auto& shape : shapes) 
	{
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
				uniqueVertices[vertex] = static_cast<uint32_t>(model.Vertices.size());
				model.Vertices.push_back(vertex);
			}

			model.Indices.push_back(uniqueVertices[vertex]);
		}
	}

	model.IsStatic = _isStatic;
	
#ifdef _DEBUG
	std::cout << "Vertices count: " << model.Vertices.size() << std::endl;
	std::cout << "Indices count: " << model.Indices.size() << std::endl;
#endif

	return std::make_unique<ModelData>(model);
}

VESPERENGINE_NAMESPACE_END
