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

	// Assign per-face color (6 sides  6 indices = 36)
	// You can now color the vertices based on which face they are part of
	// This is purely for visualization and does not affect OBJ loading

	std::array<std::array<int32, 6>, 6> faceTriangles = { {
		{{4, 5, 6, 4, 6, 7}},  // front
		{{0, 2, 1, 0, 3, 2}},  // back
		{{0, 7, 3, 0, 4, 7}},  // left
		{{1, 2, 6, 1, 6, 5}},  // right
		{{3, 7, 6, 3, 6, 2}},  // top
		{{0, 1, 5, 0, 5, 4}},  // bottom
	} };

	for (size_t i = 0; i < faceTriangles.size(); ++i)
	{
		for (int32 idx : faceTriangles[i])
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

std::unique_ptr<ModelData> PrimitiveFactory::GenerateSphere(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _color, uint32 _segments, uint32 _rings, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	for (uint32 y = 0; y <= _rings; ++y)
	{
		float v = static_cast<float>(y) / static_cast<float>(_rings);
		float theta = v * glm::pi<float>();
		for (uint32 x = 0; x <= _segments; ++x)
		{
			float u = static_cast<float>(x) / static_cast<float>(_segments);
			float phi = u * glm::two_pi<float>();

			glm::vec3 pos{
					glm::cos(phi) * glm::sin(theta),
					glm::cos(theta),
					glm::sin(phi) * glm::sin(theta)
			};

			Vertex vert{};
			vert.Position = pos + _offset;
			vert.Color = _color;
			data->Vertices.push_back(vert);
		}
	}

	for (uint32 y = 0; y < _rings; ++y)
	{
		for (uint32 x = 0; x < _segments; ++x)
		{
			uint32 first = y * (_segments + 1) + x;
			uint32 second = first + _segments + 1;

			data->Indices.push_back(first);
			data->Indices.push_back(second);
			data->Indices.push_back(first + 1);

			data->Indices.push_back(second);
			data->Indices.push_back(second + 1);
			data->Indices.push_back(first + 1);
		}
	}

	data->Material = _materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
	data->IsStatic = _isStatic;

	return data;
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateCone(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _color, uint32 _segments, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	for (uint32 i = 0; i < _segments; ++i)
	{
		float angle = static_cast<float>(i) / static_cast<float>(_segments) * glm::two_pi<float>();
		glm::vec3 pos{ glm::cos(angle) * 0.5f, -0.5f, glm::sin(angle) * 0.5f };
		Vertex vert{};
		vert.Position = pos + _offset;
		vert.Color = _color;
		data->Vertices.push_back(vert);
	}

	Vertex apex{};
	apex.Position = glm::vec3(0.0f, 0.5f, 0.0f) + _offset;
	apex.Color = _color;
	data->Vertices.push_back(apex);

	Vertex center{};
	center.Position = glm::vec3(0.0f, -0.5f, 0.0f) + _offset;
	center.Color = _color;
	data->Vertices.push_back(center);

	uint32 apexIndex = static_cast<uint32>(data->Vertices.size() - 2);
	uint32 centerIndex = static_cast<uint32>(data->Vertices.size() - 1);

	for (uint32 i = 0; i < _segments; ++i)
	{
		uint32 next = (i + 1) % _segments;

		data->Indices.push_back(i);
		data->Indices.push_back(next);
		data->Indices.push_back(apexIndex);

		data->Indices.push_back(i);
		data->Indices.push_back(centerIndex);
		data->Indices.push_back(next);
	}

	data->Material = _materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
	data->IsStatic = _isStatic;

	return data;
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateParallelepiped(MaterialSystem& _materialSystem, glm::vec3 _offset, float bottomSize, float topSize, float height, glm::vec3 _facesColor, bool _isStatic)
{
	return GenerateParallelepiped(_materialSystem, _offset, bottomSize, topSize, height, { _facesColor , _facesColor , _facesColor , _facesColor , _facesColor , _facesColor }, _isStatic);
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateParallelepiped(MaterialSystem& _materialSystem, glm::vec3 _offset, float bottomSize, float topSize, float height, std::array<glm::vec3, 6> _facesColor, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	float b = bottomSize * 0.5f;
	float t = topSize * 0.5f;
	float h = height;

	// Define 8 vertices: 4 bottom, 4 top
	std::vector<Vertex> vertices = {
		{{-b, 0.0f, -b}}, // 0: bottom back left
		{{ b, 0.0f, -b}}, // 1: bottom back right
		{{ b, 0.0f,  b}}, // 2: bottom front right
		{{-b, 0.0f,  b}}, // 3: bottom front left

		{{-t, h, -t}}, // 4: top back left
		{{ t, h, -t}}, // 5: top back right
		{{ t, h,  t}}, // 6: top front right
		{{-t, h,  t}}, // 7: top front left
	};

	// Apply offset
	for (auto& v : vertices) {
		v.Position += _offset;
		v.Color = glm::vec3(1.0f); // default color
	}

	data->Vertices = vertices;

	// Define indices (CCW for each face)
	data->Indices = {
		// Bottom face (0, 1, 2, 3)
		0, 2, 1,
		0, 3, 2,

		// Top face (4, 5, 6, 7)
		4, 5, 6,
		4, 6, 7,

		// Front face (3, 2, 6, 7)
		3, 6, 2,
		3, 7, 6,

		// Back face (0, 1, 5, 4)
		0, 1, 5,
		0, 5, 4,

		// Left face (0, 4, 7, 3)
		0, 4, 7,
		0, 7, 3,

		// Right face (1, 2, 6, 5)
		1, 2, 6,
		1, 6, 5,
	};

	// Face index map for coloring: 6 faces, 6 triangle groups
	std::array<std::array<int32, 6>, 6> faceTriangles = { {
		{{0, 2, 1, 0, 3, 2}}, // bottom
		{{4, 5, 6, 4, 6, 7}}, // top
		{{3, 6, 2, 3, 7, 6}}, // front
		{{0, 1, 5, 0, 5, 4}}, // back
		{{0, 4, 7, 0, 7, 3}}, // left
		{{1, 2, 6, 1, 6, 5}}, // right
	} };

	for (size_t i = 0; i < faceTriangles.size(); ++i)
	{
		for (int32 idx : faceTriangles[i])
		{
			data->Vertices[data->Indices[idx]].Color = _facesColor[i];
		}
	}

	data->Material = _materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
	data->IsStatic = _isStatic;

	LOG(Logger::INFO, "Model: Parallelepiped (bottom=", bottomSize, ", top=", topSize, ", height=", height, ")");
	LOG(Logger::INFO, "Vertices count: ", data->Vertices.size());
	LOG(Logger::INFO, "Indices count: ", data->Indices.size());
	LOG_NL();

	return data;
}


VESPERENGINE_NAMESPACE_END