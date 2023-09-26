#pragma once

#include "Core/core_defines.h"
#include "Backend/model_data.h"

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <array>
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL PrimitiveFactory
{
public:
	static std::unique_ptr<ModelData> GenerateTriangleNoIndices(glm::vec3 _offset, glm::vec3 _faceColor, bool _isStatic = true);
	static std::unique_ptr<ModelData> GenerateTriangle(glm::vec3 _offset, glm::vec3 _faceColor, bool _isStatic = true);

	static std::unique_ptr<ModelData> GenerateCubeNoIndices(glm::vec3 _offset, glm::vec3 _facesColor, bool _isStatic = true);
	static std::unique_ptr<ModelData> GenerateCubeNoIndices(glm::vec3 _offset, std::array<glm::vec3, 6> _facesColor, bool _isStatic = true);

	static std::unique_ptr<ModelData> GenerateCube(glm::vec3 _offset, glm::vec3 _facesColor, bool _isStatic = true);
	static std::unique_ptr<ModelData> GenerateCube(glm::vec3 _offset, std::array<glm::vec3, 6> _facesColor, bool _isStatic = true);

private:
	PrimitiveFactory() = delete;
	~PrimitiveFactory() = delete;

	PrimitiveFactory(const PrimitiveFactory&) = delete;
	PrimitiveFactory& operator=(const PrimitiveFactory&) = delete;
};

VESPERENGINE_NAMESPACE_END
