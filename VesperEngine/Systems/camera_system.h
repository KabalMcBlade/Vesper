// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\camera_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Components/camera_components.h"
#include "Components/object_components.h"
#include "Components/graphics_components.h"

#include "ECS/ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;

class VESPERENGINE_API CameraSystem
{
public:
	CameraSystem(VesperApp& _app);
	~CameraSystem() = default;

	CameraSystem(const CameraSystem&) = delete;
	CameraSystem& operator=(const CameraSystem&) = delete;

public:
	void SetCurrentActiveCamera(ecs::Entity _activeCamera);
	void SwitchActiveCamera();

	void Update(const float _aspectRatio);
	void GetActiveCameraData(const uint32 _activeCameraIndex, CameraComponent& _outCameraComponent, CameraTransformComponent& _outCameraTransform);

	void SetOrthographicProjection(CameraComponent& _camera, float _left, float _right, float _top, float _bottom, float _near, float _far) const;
	void SetPerspectiveProjection(CameraComponent& _camera, float _fovY, float _aspectRatio, float _near, float _far) const;

	void SetViewDirection(CameraComponent& _camera, const glm::vec3& _position, glm::vec3 _direction, glm::vec3 _up = { 0.0f, -1.0f, 0.0f }) const;
	void SetViewTarget(CameraComponent& _camera, const glm::vec3& _position, glm::vec3 _target, glm::vec3 _up = { 0.0f, -1.0f, 0.0f }) const;
	void SetViewRotation(CameraComponent& _camera, const glm::vec3& _position, const float _yaw, const float _pitch, const float _roll) const;
	void SetViewRotation(CameraComponent& _camera, const CameraTransformComponent& _transform) const;

private:
	VesperApp& m_app;
	int32 m_lastCameraActiveIndex{ 0 };
};

VESPERENGINE_NAMESPACE_END
