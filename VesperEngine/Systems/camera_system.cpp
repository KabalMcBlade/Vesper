// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\camera_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/camera_system.h"

#include "Components/camera_components.h"
#include "Components/object_components.h"

#include "App/vesper_app.h"

#include "ECS/ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

CameraSystem::CameraSystem(VesperApp& _app)
	: m_app(_app)
{
}

void CameraSystem::SetCurrentActiveCamera(ecs::Entity _activeCamera)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	if (!componentManager.HasComponents<CameraComponent>(_activeCamera))
	{
		return;
	}

	if (componentManager.HasComponents<CameraActive>(_activeCamera))
	{
		return;
	}
	
	// TODO: maybe use collect here? Might alter current iterator, double check when having more than 1 camera
	for (auto camera : ecs::IterateEntitiesWithAll<CameraComponent, CameraActive>(entityManager, componentManager))
	{
		componentManager.RemoveComponent<CameraActive>(camera);
	}
	
	componentManager.AddComponent<CameraActive>(_activeCamera);
}

void CameraSystem::SwitchActiveCamera()
{
	std::vector<ecs::Entity> cameras = ecs::EntityCollector::CollectEntitiesWithAll<CameraTransformComponent>(m_app.GetEntityManager(), m_app.GetComponentManager());

	const int32 cameraCount = static_cast<int32>(cameras.size());
	for (int32 i = 0; i < cameraCount; ++i)
	{
		ecs::Entity entity = cameras[i];

		if (m_app.GetComponentManager().HasComponents<CameraActive>(entity))
		{
			m_lastCameraActiveIndex = i;
			break;
		}
	}

	const int32 cameraIndexToActive = (m_lastCameraActiveIndex + 1) % cameraCount;

	SetCurrentActiveCamera(cameras[cameraIndexToActive]);

	cameras.clear();
}

void CameraSystem::Update(const float _aspectRatio)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto camera : ecs::IterateEntitiesWithAll<CameraComponent, CameraTransformComponent>(entityManager, componentManager))
	{
		CameraTransformComponent& transformComponent = componentManager.GetComponent<CameraTransformComponent>(camera);
		CameraComponent& cameraComponent = componentManager.GetComponent<CameraComponent>(camera);

		SetViewRotation(cameraComponent, transformComponent);
		SetPerspectiveProjection(cameraComponent, glm::radians(50.0f), _aspectRatio, 0.1f, 100.0f);
	}
}

void CameraSystem::GetActiveCameraData(const uint32 _activeCameraIndex, CameraComponent& _outCameraComponent, CameraTransformComponent& _outCameraTransform)
{
	std::vector<ecs::Entity> cameras = ecs::EntityCollector::CollectEntitiesWithAny<CameraActive, CameraComponent, CameraTransformComponent>(m_app.GetEntityManager(), m_app.GetComponentManager());

	assertMsgReturnVoid(cameras.size() > 0, "There is no active camera!");
	assertMsgReturnVoid(_activeCameraIndex >= 0 && _activeCameraIndex < cameras.size(), "Active camera index is out of bound!");

	const ecs::Entity activeCamera = cameras[_activeCameraIndex];

	_outCameraComponent = m_app.GetComponentManager().GetComponent<CameraComponent>(activeCamera);
	_outCameraTransform = m_app.GetComponentManager().GetComponent<CameraTransformComponent>(activeCamera);

	cameras.clear();
}

void CameraSystem::SetOrthographicProjection(CameraComponent& _camera, float _left, float _right, float _top, float _bottom, float _near, float _far) const
{
	_camera.ProjectionMatrix = glm::orthoLH_ZO(_left, _right, _bottom, _top, _near, _far);
}

void CameraSystem::SetPerspectiveProjection(CameraComponent& _camera, float _fovY, float _aspectRatio, float _near, float _far) const
{
	assertMsgReturnVoid(glm::abs(_aspectRatio - std::numeric_limits<float>::epsilon()) > 0.0f, "Aspect ratio is not valid");
	_camera.ProjectionMatrix = glm::perspectiveLH_ZO(_fovY, _aspectRatio, _near, _far);
}

void CameraSystem::SetViewDirection(CameraComponent& _camera, const glm::vec3 _position, const glm::vec3 _direction, const glm::vec3 _up) const
{
	const glm::vec3 target = _position + _direction;
	_camera.ViewMatrix = glm::lookAtLH(_position, target, _up);
}

void CameraSystem::SetViewTarget(CameraComponent& _camera, const glm::vec3 _position, const glm::vec3 _target, const glm::vec3 _up) const
{
	assertMsgReturnVoid(glm::length(_target - _position) > 0.0f, "The direction cannot be 0");
	SetViewDirection(_camera, _position, _target - _position, _up);
}

void CameraSystem::SetViewRotation(CameraComponent& _camera, const glm::vec3 _position, const float _yaw, const float _pitch, const float _roll) const
{
	// Build transform: rotate then translate
	glm::mat4 rotation = glm::yawPitchRoll(_yaw, _pitch, _roll);
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), -_position);
	_camera.ViewMatrix = glm::inverse(rotation) * translation;
}

void CameraSystem::SetViewRotation(CameraComponent& _camera, const CameraTransformComponent& _transform) const
{
	SetViewRotation(_camera, _transform.Position, _transform.Rotation.y, _transform.Rotation.x, _transform.Rotation.z);
}


VESPERENGINE_NAMESPACE_END
