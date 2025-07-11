// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\uniform_buffer.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"


VESPERENGINE_NAMESPACE_BEGIN

// Scene
struct VESPERENGINE_ALIGN16 SceneUBO
{
	glm::mat4 ProjectionMatrix{ 1.0f };
	glm::mat4 ViewMatrix{ 1.0f };
	glm::vec4 CameraPosition{ 0.0f, 0.0f, 0.0f, 1.0f };
	glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 0.3f };	// w is intensity
};

// Directional Light
struct VESPERENGINE_ALIGN16 DirectionalLight
{
	glm::vec4 Direction{ 0.0f, -1.0f, 0.0f, 0.0f };
	glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f }; // w intensity
};

// Point Light
struct VESPERENGINE_ALIGN16 PointLight
{
	glm::vec4 Position{ 0.0f, 0.0f, 0.0f, 0.0f };
	glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f }; // w intensity
	glm::vec4 Attenuation{ 1.0f, 0.0f, 0.0f, 0.0f }; // constant, linear, quadratic
};

// Spot Light
struct VESPERENGINE_ALIGN16 SpotLight
{
	glm::vec4 Position{ 0.0f, 0.0f, 0.0f, 0.0f };
	glm::vec4 Direction{ 0.0f, -1.0f, 0.0f, 0.0f };
	glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f }; // w intensity
	glm::vec4 Params{ 0.8f, 0.9f, 0.0f, 0.0f }; // x innerCutoff, y outerCutoff
};

static constexpr uint32 kMaxDirectionalLights = 16;
static constexpr uint32 kMaxPointLights = 256;
static constexpr uint32 kMaxSpotLights = 256;
static constexpr uint32 kMaxMorphTargets = 8;

struct VESPERENGINE_ALIGN16 LightsUBO
{
	int32 DirectionalCount{ 0 };
	int32 PointCount{ 0 };
	int32 SpotCount{ 0 };
	int32 _padding;
	DirectionalLight DirectionalLights[kMaxDirectionalLights];
	PointLight PointLights[kMaxPointLights];
	SpotLight SpotLights[kMaxSpotLights];
};

// Entity
struct VESPERENGINE_ALIGN16 EntityUBO
{
	glm::mat4 ModelMatrix{ 1.0f };
	glm::vec4 MorphWeights0{ 0.0f };
	glm::vec4 MorphWeights1{ 0.0f };
	int32 MorphTargetCount{ 0 };
};


struct VESPERENGINE_ALIGN16 PhongMaterialUBO
{
	glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 DiffuseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 SpecularColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	glm::vec4 EmissionColor{ 0.0f, 0.0f, 0.0f, 1.0f };

	alignFloat Shininess{ 0.5f };

	// Indices for [Ambient, Diffuse, Specular, Normal, Alpha]
	alignInt32 TextureIndices[5] = { -1, -1, -1, -1, -1 };
	// UV set index for each texture above (0 = UV1, 1 = UV2)
	alignInt32 UVIndices[5] = { 0, 0, 0, 0, 0 };
};

struct VESPERENGINE_ALIGN16 PBRMaterialUBO
{
	float Roughness{ 0.0f };
	float Metallic{ 0.0f };
	float Sheen{ 0.0f };
	float ClearcoatThickness{ 0.0f };
	float ClearcoatRoughness{ 0.0f };
	float Anisotropy{ 0.0f };
	float AnisotropyRotation{ 0.0f };
	float AlphaCutoff{ -1.0f };        // < 0 means disabled
	float BaseColorAlpha{ 1.0f };

	// Indices for [Roughness, Metallic, Sheen, Emissive, Normal, BaseColor, AmbientOcclusion]
	alignInt32 TextureIndices[7] = { -1, -1, -1, -1, -1, -1, -1 }; 
	// UV set index for each texture above (0 = UV1, 1 = UV2)
	alignInt32 UVIndices[7] = { 0, 0, 0, 0, 0, 0, 0 };
};


struct VESPERENGINE_ALIGN16 PhongBindlessMaterialIndexUBO
{
	alignInt32 MaterialIndex{ 0 };
};

VESPERENGINE_NAMESPACE_END