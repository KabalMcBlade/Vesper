#include "primitive_factory.h"

#include "Utility/logger.h"
#include "Systems/material_system.h"

VESPERENGINE_NAMESPACE_BEGIN

std::unique_ptr<ModelData> PrimitiveFactory::GenerateTriangleNoIndices(glm::vec3 _offset, glm::vec3 _faceColor, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	data->Vertices =
	{
		{{0.0f, -0.5f, 0.0f}, _faceColor},
		{{0.5f, 0.5f, 0.0f}, _faceColor},
		{{-0.5f, 0.5f, 0.0f}, _faceColor}
	};

	for (auto& v : data->Vertices)
	{
		v.Position += _offset;
	}

	data->Material = MaterialSystem::CreateDefaultPhongMaterialData();

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

std::unique_ptr<ModelData> PrimitiveFactory::GenerateTriangle(glm::vec3 _offset, glm::vec3 _faceColor, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	data->Vertices =
	{
		{{0.0f, -0.5f, 0.0f}, _faceColor},
		{{0.5f, 0.5f, 0.0f}, _faceColor},
		{{-0.5f, 0.5f, 0.0f}, _faceColor}
	};

	for (auto& v : data->Vertices)
	{
		v.Position += _offset;
	}

	data->Indices = { 0,  1,  2 };

	data->Material = MaterialSystem::CreateDefaultPhongMaterialData();

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

std::unique_ptr<ModelData> PrimitiveFactory::GenerateCubeNoIndices(glm::vec3 _offset, glm::vec3 _facesColor, bool _isStatic)
{
	return GenerateCubeNoIndices(_offset, { _facesColor , _facesColor , _facesColor , _facesColor , _facesColor , _facesColor }, _isStatic);
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateCubeNoIndices(glm::vec3 _offset, std::array<glm::vec3, 6> _facesColor, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();

	data->Vertices =
	{
		// left face
		{{-.5f, -.5f, -.5f}, _facesColor[0]},
		{{-.5f, .5f, .5f}, _facesColor[0]},
		{{-.5f, -.5f, .5f}, _facesColor[0]},
		{{-.5f, -.5f, -.5f}, _facesColor[0]},
		{{-.5f, .5f, -.5f}, _facesColor[0]},
		{{-.5f, .5f, .5f}, _facesColor[0]},

		// right face
		{{.5f, -.5f, -.5f}, _facesColor[1]},
		{{.5f, .5f, .5f},_facesColor[1]},
		{{.5f, -.5f, .5f},_facesColor[1]},
		{{.5f, -.5f, -.5f},_facesColor[1]},
		{{.5f, .5f, -.5f},_facesColor[1]},
		{{.5f, .5f, .5f},_facesColor[1]},

		// top face
		{{-.5f, -.5f, -.5f}, _facesColor[2]},
		{{.5f, -.5f, .5f}, _facesColor[2]},
		{{-.5f, -.5f, .5f}, _facesColor[2]},
		{{-.5f, -.5f, -.5f}, _facesColor[2]},
		{{.5f, -.5f, -.5f}, _facesColor[2]},
		{{.5f, -.5f, .5f}, _facesColor[2]},

		// bottom face
		{{-.5f, .5f, -.5f}, _facesColor[3]},
		{{.5f, .5f, .5f}, _facesColor[3]},
		{{-.5f, .5f, .5f}, _facesColor[3]},
		{{-.5f, .5f, -.5f}, _facesColor[3]},
		{{.5f, .5f, -.5f}, _facesColor[3]},
		{{.5f, .5f, .5f}, _facesColor[3]},

		// nose face
		{{-.5f, -.5f, 0.5f}, _facesColor[4]},
		{{.5f, .5f, 0.5f}, _facesColor[4]},
		{{-.5f, .5f, 0.5f}, _facesColor[4]},
		{{-.5f, -.5f, 0.5f}, _facesColor[4]},
		{{.5f, -.5f, 0.5f}, _facesColor[4]},
		{{.5f, .5f, 0.5f}, _facesColor[4]},

		// tail face 
		{{-.5f, -.5f, -0.5f}, _facesColor[5]},
		{{.5f, .5f, -0.5f}, _facesColor[5]},
		{{-.5f, .5f, -0.5f}, _facesColor[5]},
		{{-.5f, -.5f, -0.5f}, _facesColor[5]},
		{{.5f, -.5f, -0.5f}, _facesColor[5]},
		{{.5f, .5f, -0.5f}, _facesColor[5]},

	};

	for (auto& v : data->Vertices)
	{
		v.Position += _offset;
	}

	data->Material = MaterialSystem::CreateDefaultPhongMaterialData();

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


std::unique_ptr<ModelData> PrimitiveFactory::GenerateCube(glm::vec3 _offset, glm::vec3 _facesColor, bool _isStatic)
{
	return GenerateCube(_offset, { _facesColor , _facesColor , _facesColor , _facesColor , _facesColor , _facesColor }, _isStatic);
}

std::unique_ptr<ModelData> PrimitiveFactory::GenerateCube(glm::vec3 _offset, std::array<glm::vec3, 6> _facesColor, bool _isStatic)
{
	auto data = std::make_unique<ModelData>();
	
	data->Vertices =
	{
		// left face
		{{-.5f, -.5f, -.5f}, _facesColor[0]},
		{{-.5f, .5f, .5f}, _facesColor[0]},
		{{-.5f, -.5f, .5f}, _facesColor[0]},
		{{-.5f, .5f, -.5f}, _facesColor[0]},

		// right face
		{{.5f, -.5f, -.5f}, _facesColor[1]},
		{{.5f, .5f, .5f},_facesColor[1]},
		{{.5f, -.5f, .5f},_facesColor[1]},
		{{.5f, .5f, -.5f},_facesColor[1]},

		// top face
		{{-.5f, -.5f, -.5f}, _facesColor[2]},
		{{.5f, -.5f, .5f}, _facesColor[2]},
		{{-.5f, -.5f, .5f}, _facesColor[2]},
		{{.5f, -.5f, -.5f}, _facesColor[2]},

		// bottom face
		{{-.5f, .5f, -.5f}, _facesColor[3]},
		{{.5f, .5f, .5f}, _facesColor[3]},
		{{-.5f, .5f, .5f}, _facesColor[3]},
		{{.5f, .5f, -.5f}, _facesColor[3]},

		// nose face
		{{-.5f, -.5f, 0.5f}, _facesColor[4]},
		{{.5f, .5f, 0.5f}, _facesColor[4]},
		{{-.5f, .5f, 0.5f}, _facesColor[4]},
		{{.5f, -.5f, 0.5f}, _facesColor[4]},

		// tail face 
		{{-.5f, -.5f, -0.5f}, _facesColor[5]},
		{{.5f, .5f, -0.5f}, _facesColor[5]},
		{{-.5f, .5f, -0.5f}, _facesColor[5]},
		{{.5f, -.5f, -0.5f}, _facesColor[5]},

	};

	for (auto& v : data->Vertices)
	{
		v.Position += _offset;
	}

	data->Indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
					12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

	data->Material = MaterialSystem::CreateDefaultPhongMaterialData();

	data->IsStatic = _isStatic;

	LOG(Logger::INFO, "Model: Cube");
	LOG(Logger::INFO, "Vertices count: ", data->Vertices.size());
	LOG(Logger::INFO, "Indices count: ", data->Indices.size());
	LOG_NL();

	LOG(Logger::INFO, "Total shapes processed: 1");
	LOG_NL();
	LOG_NL();

	return data;
}

VESPERENGINE_NAMESPACE_END
