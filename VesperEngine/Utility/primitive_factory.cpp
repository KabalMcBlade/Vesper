// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\primitive_factory.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Utility/primitive_factory.h"
#include "Utility/logger.h"

#include "Backend/buffer.h"
#include "Backend/model_data.h"

#include "Systems/material_system.h"


VESPERENGINE_NAMESPACE_BEGIN

std::unique_ptr<ModelData> PrimitiveFactory::GenerateTriangleNoIndices(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _faceColor, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	// Single triangle, CCW winding on XY plane (Z = 0)
	std::vector<Vertex> vertices = {
		{{ 0.0f, -0.5f, 0.0f}, _faceColor}, // bottom center
		{{ 0.5f,  0.5f, 0.0f}, _faceColor}, // top right
		{{-0.5f,  0.5f, 0.0f}, _faceColor}, // top left
	};

	for (auto& v : vertices)
	{
		v.Position += _offset;
	}

	data->Vertices = std::move(vertices);
	data->Material = _materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
	data->IsStatic = _isStatic;

	LOG(Logger::INFO, "Model: TriangleNoIndices");
	LOG(Logger::INFO, "Vertices count: ", data->Vertices.size());
	LOG(Logger::INFO, "Indices count: ", data->Indices.size());
	LOG_NL();
	LOG(Logger::INFO, "Total shapes processed: 1");
	LOG_NL();
	LOG_NL();

	return data;
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateTriangle(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _faceColor, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	std::vector<Vertex> vertices = {
		{{ 0.0f, -0.5f, 0.0f}, _faceColor}, // bottom center
		{{ 0.5f,  0.5f, 0.0f}, _faceColor}, // top right
		{{-0.5f,  0.5f, 0.0f}, _faceColor}, // top left
	};

	for (auto& v : vertices)
	{
		v.Position += _offset;
	}

	data->Vertices = std::move(vertices);
	data->Indices = { 0, 1, 2 }; // CCW order

	data->Material = _materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
	data->IsStatic = _isStatic;

	LOG(Logger::INFO, "Model: Triangle");
	LOG(Logger::INFO, "Vertices count: ", data->Vertices.size());
	LOG(Logger::INFO, "Indices count: ", data->Indices.size());
	LOG_NL();
	LOG(Logger::INFO, "Total shapes processed: 1");
	LOG_NL();
	LOG_NL();

	return data;
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateCubeNoIndices(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _facesColor, bool _isStatic)
{
	return GenerateCubeNoIndices(_materialSystem, _offset, { _facesColor , _facesColor , _facesColor , _facesColor , _facesColor , _facesColor }, _isStatic);
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateCubeNoIndices(MaterialSystem& _materialSystem, glm::vec3 _offset, std::array<glm::vec3, 6> _facesColor, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	// Each face consists of 2 triangles (6 vertices), ordered CCW
	std::vector<Vertex> vertices = {
		// Front (+Z)
		{{-0.5f, -0.5f,  0.5f}, _facesColor[0]},
		{{ 0.5f,  0.5f,  0.5f}, _facesColor[0]},
		{{-0.5f,  0.5f,  0.5f}, _facesColor[0]},
		{{-0.5f, -0.5f,  0.5f}, _facesColor[0]},
		{{ 0.5f, -0.5f,  0.5f}, _facesColor[0]},
		{{ 0.5f,  0.5f,  0.5f}, _facesColor[0]},

		// Back (-Z)
		{{-0.5f, -0.5f, -0.5f}, _facesColor[1]},
		{{-0.5f,  0.5f, -0.5f}, _facesColor[1]},
		{{ 0.5f,  0.5f, -0.5f}, _facesColor[1]},
		{{-0.5f, -0.5f, -0.5f}, _facesColor[1]},
		{{ 0.5f,  0.5f, -0.5f}, _facesColor[1]},
		{{ 0.5f, -0.5f, -0.5f}, _facesColor[1]},

		// Left (-X)
		{{-0.5f, -0.5f, -0.5f}, _facesColor[2]},
		{{-0.5f,  0.5f,  0.5f}, _facesColor[2]},
		{{-0.5f,  0.5f, -0.5f}, _facesColor[2]},
		{{-0.5f, -0.5f, -0.5f}, _facesColor[2]},
		{{-0.5f, -0.5f,  0.5f}, _facesColor[2]},
		{{-0.5f,  0.5f,  0.5f}, _facesColor[2]},

		// Right (+X)
		{{ 0.5f, -0.5f, -0.5f}, _facesColor[3]},
		{{ 0.5f,  0.5f, -0.5f}, _facesColor[3]},
		{{ 0.5f,  0.5f,  0.5f}, _facesColor[3]},
		{{ 0.5f, -0.5f, -0.5f}, _facesColor[3]},
		{{ 0.5f,  0.5f,  0.5f}, _facesColor[3]},
		{{ 0.5f, -0.5f,  0.5f}, _facesColor[3]},

		// Top (+Y)
		{{-0.5f,  0.5f, -0.5f}, _facesColor[4]},
		{{-0.5f,  0.5f,  0.5f}, _facesColor[4]},
		{{ 0.5f,  0.5f,  0.5f}, _facesColor[4]},
		{{-0.5f,  0.5f, -0.5f}, _facesColor[4]},
		{{ 0.5f,  0.5f,  0.5f}, _facesColor[4]},
		{{ 0.5f,  0.5f, -0.5f}, _facesColor[4]},

		// Bottom (-Y)
		{{-0.5f, -0.5f, -0.5f}, _facesColor[5]},
		{{ 0.5f, -0.5f,  0.5f}, _facesColor[5]},
		{{-0.5f, -0.5f,  0.5f}, _facesColor[5]},
		{{-0.5f, -0.5f, -0.5f}, _facesColor[5]},
		{{ 0.5f, -0.5f, -0.5f}, _facesColor[5]},
		{{ 0.5f, -0.5f,  0.5f}, _facesColor[5]},
	};

	// Apply the offset to all positions
	for (auto& v : vertices)
	{
		v.Position += _offset;
	}

	data->Vertices = std::move(vertices);

	data->Material = _materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
	data->IsStatic = _isStatic;

	LOG(Logger::INFO, "Model: CubeNoIndices");
	LOG(Logger::INFO, "Vertices count: ", data->Vertices.size());
	LOG(Logger::INFO, "Indices count: ", data->Indices.size());
	LOG_NL();
	LOG(Logger::INFO, "Total shapes processed: 1");
	LOG_NL();
	LOG_NL();

	return data;
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateCube(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _facesColor, bool _isStatic)
{
	return GenerateCube(_materialSystem, _offset, { _facesColor , _facesColor , _facesColor , _facesColor , _facesColor , _facesColor }, _isStatic);
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateCube(MaterialSystem& _materialSystem, glm::vec3 _offset, std::array<glm::vec3, 6> _facesColor, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	// Unique 8 cube corners, shared across faces
	std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, -0.5f}}, // 0
		{{ 0.5f, -0.5f, -0.5f}}, // 1
		{{ 0.5f,  0.5f, -0.5f}}, // 2
		{{-0.5f,  0.5f, -0.5f}}, // 3
		{{-0.5f, -0.5f,  0.5f}}, // 4
		{{ 0.5f, -0.5f,  0.5f}}, // 5
		{{ 0.5f,  0.5f,  0.5f}}, // 6
		{{-0.5f,  0.5f,  0.5f}}, // 7
	};

	// Apply offset and assign color (one color per vertex based on which face owns it most)
	for (auto& v : vertices)
	{
		v.Position += _offset;
		v.Color = glm::vec3(1.0f); // default white, or assign a default
	}

	data->Vertices = vertices;

	// Define indices using CCW order for each face and assign per-face color manually below
	data->Indices = {
		// Front (Z+)
		4, 5, 6,
		4, 6, 7,

		// Back (Z-)
		0, 2, 1,
		0, 3, 2,

		// Left (X-)
		0, 7, 3,
		0, 4, 7,

		// Right (X+)
		1, 2, 6,
		1, 6, 5,

		// Top (Y+)
		3, 7, 6,
		3, 6, 2,

		// Bottom (Y-)
		0, 1, 5,
		0, 5, 4,
	};

	// Assign per-face color (6 sides × 6 indices = 36)
	// You can now color the vertices based on which face they are part of
	// This is purely for visualization and does not affect OBJ loading

	std::array<std::array<int, 6>, 6> faceTriangles = { {
		{{4, 5, 6, 4, 6, 7}},  // front
		{{0, 2, 1, 0, 3, 2}},  // back
		{{0, 7, 3, 0, 4, 7}},  // left
		{{1, 2, 6, 1, 6, 5}},  // right
		{{3, 7, 6, 3, 6, 2}},  // top
		{{0, 1, 5, 0, 5, 4}},  // bottom
	} };

	for (size_t i = 0; i < faceTriangles.size(); ++i)
	{
		for (int idx : faceTriangles[i])
		{
			data->Vertices[idx].Color = _facesColor[i];
		}
	}

	data->Material = _materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
	data->IsStatic = _isStatic;

	LOG(Logger::INFO, "Model: Cube");
	LOG(Logger::INFO, "Vertices count: ", data->Vertices.size());
	LOG(Logger::INFO, "Indices count: ", data->Indices.size());
	LOG_NL();

	return data;
}

VESPERENGINE_NAMESPACE_END
