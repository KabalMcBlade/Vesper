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
	glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 0.3f };	// w is intensity
};

// Light
struct VESPERENGINE_ALIGN16 LightUBO
{
	glm::vec4 LightPos{ 0.0f, -0.25f, 0.0f, 0.0f };
	glm::vec4 LightColor{ 1.0f, 1.0f, 1.0f, 0.5f };
};

// Entity
struct VESPERENGINE_ALIGN16 EntityUBO
{
	glm::mat4 ModelMatrix{ 1.0f };
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

	// Indices for [Roughness, Metallic, Sheen, Emissive, Normal, BaseColor, AmbientOcclusion]
	alignInt32 TextureIndices[7] = { -1, -1, -1, -1, -1, -1, -1 }; 
};


struct VESPERENGINE_ALIGN16 PhongBindlessMaterialIndexUBO
{
	alignInt32 MaterialIndex{ 0 };
};

VESPERENGINE_NAMESPACE_END