// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\primitive_factory.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Backend/model_data.h"

#include "Systems/material_system.h"

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <array>
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API PrimitiveFactory
{
public:
	static std::unique_ptr<ModelData> GenerateTriangleNoIndices(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _faceColor, bool _isStatic = true);
	static std::unique_ptr<ModelData> GenerateTriangle(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _faceColor, bool _isStatic = true);

	static std::unique_ptr<ModelData> GenerateCubeNoIndices(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _facesColor, bool _isStatic = true);
	static std::unique_ptr<ModelData> GenerateCubeNoIndices(MaterialSystem& _materialSystem, glm::vec3 _offset, std::array<glm::vec3, 6> _facesColor, bool _isStatic = true);

	static std::unique_ptr<ModelData> GenerateCube(MaterialSystem& _materialSystem, glm::vec3 _offset, glm::vec3 _facesColor, bool _isStatic = true);
	static std::unique_ptr<ModelData> GenerateCube(MaterialSystem& _materialSystem, glm::vec3 _offset, std::array<glm::vec3, 6> _facesColor, bool _isStatic = true);

private:
	PrimitiveFactory() = delete;
	~PrimitiveFactory() = delete;

	PrimitiveFactory(const PrimitiveFactory&) = delete;
	PrimitiveFactory& operator=(const PrimitiveFactory&) = delete;
};

VESPERENGINE_NAMESPACE_END
