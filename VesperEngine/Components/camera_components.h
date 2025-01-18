// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Components\camera_components.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"


VESPERENGINE_NAMESPACE_BEGIN

struct CameraComponent
{
	glm::mat4 ProjectionMatrix{ 1.0f };
	glm::mat4 ViewMatrix{ 1.0f };
};

// empty struct, used to mark the current active camera (from player point of view)
struct CameraActive
{

};

// Special transform struct for camera only
struct CameraTransformComponent
{
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };
};

VESPERENGINE_NAMESPACE_END
