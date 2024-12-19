#include "camera_system.h"

#include "App/vesper_app.h"

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <gtx/quaternion.hpp>
#include <gtx/euler_angles.hpp>


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
	std::vector<ecs::Entity> cameras;
	ecs::EntityCollector::CollectEntitiesWithAll<CameraTransformComponent>(m_app.GetEntityManager(), m_app.GetComponentManager(), cameras);

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
	std::vector<ecs::Entity> cameras;
	ecs::EntityCollector::CollectEntitiesWithAny<CameraActive, CameraComponent, CameraTransformComponent>(m_app.GetEntityManager(), m_app.GetComponentManager(), cameras);

	assertMsgReturnVoid(cameras.size() > 0, "There is no active camera!");
	assertMsgReturnVoid(_activeCameraIndex >= 0 && _activeCameraIndex < cameras.size(), "Active camera index is out of bound!");

	const ecs::Entity activeCamera = cameras[_activeCameraIndex];

	_outCameraComponent = m_app.GetComponentManager().GetComponent<CameraComponent>(activeCamera);
	_outCameraTransform = m_app.GetComponentManager().GetComponent<CameraTransformComponent>(activeCamera);

	cameras.clear();
}

void CameraSystem::SetOrthographicProjection(CameraComponent& _camera, float _left, float _right, float _top, float _bottom, float _near, float _far) const
{
	_camera.ProjectionMatrix = glm::mat4{ 1.0f };
	_camera.ProjectionMatrix[0][0] = 2.f / (_right - _left);
	_camera.ProjectionMatrix[1][1] = 2.f / (_bottom - _top);
	_camera.ProjectionMatrix[2][2] = 1.f / (_far - _near);
	_camera.ProjectionMatrix[3][0] = -(_right + _left) / (_right - _left);
	_camera.ProjectionMatrix[3][1] = -(_bottom + _top) / (_bottom - _top);
	_camera.ProjectionMatrix[3][2] = -_near / (_far - _near);
}

void CameraSystem::SetPerspectiveProjection(CameraComponent& _camera, float _fovY, float _aspectRatio, float _near, float _far) const
{
	assertMsgReturnVoid(glm::abs(_aspectRatio - std::numeric_limits<float>::epsilon()) > 0.0f, "Aspect ratio is not valid");

	const float tanHalfFovy = tan(_fovY / 2.f);
	_camera.ProjectionMatrix = glm::mat4{ 0.0f };
	_camera.ProjectionMatrix[0][0] = 1.f / (_aspectRatio * tanHalfFovy);
	_camera.ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
	_camera.ProjectionMatrix[2][2] = _far / (_far - _near);
	_camera.ProjectionMatrix[2][3] = 1.f;
	_camera.ProjectionMatrix[3][2] = -(_far * _near) / (_far - _near);
}

void CameraSystem::SetViewDirection(CameraComponent& _camera, const glm::vec3& _position, glm::vec3 _direction, glm::vec3 _up) const
{
	// same as glm::lookAt, but have direction directly
	const glm::vec3 w{ glm::normalize(_direction) };
	const glm::vec3 u{ glm::normalize(glm::cross(w, _up)) };
	const glm::vec3 v{ glm::cross(w, u) };

	_camera.ViewMatrix = glm::mat4{ 1.f };
	_camera.ViewMatrix[0][0] = u.x;
	_camera.ViewMatrix[1][0] = u.y;
	_camera.ViewMatrix[2][0] = u.z;
	_camera.ViewMatrix[0][1] = v.x;
	_camera.ViewMatrix[1][1] = v.y;
	_camera.ViewMatrix[2][1] = v.z;
	_camera.ViewMatrix[0][2] = w.x;
	_camera.ViewMatrix[1][2] = w.y;
	_camera.ViewMatrix[2][2] = w.z;
	_camera.ViewMatrix[3][0] = -glm::dot(u, _position);
	_camera.ViewMatrix[3][1] = -glm::dot(v, _position);
	_camera.ViewMatrix[3][2] = -glm::dot(w, _position);
}

void CameraSystem::SetViewTarget(CameraComponent& _camera, const glm::vec3& _position, glm::vec3 _target, glm::vec3 _up) const
{
	assertMsgReturnVoid(glm::length(_target - _position) > 0.0f, "The direction cannot be 0");
	
	// same as glm::lookAt
	SetViewDirection(_camera, _position, _target - _position, _up);
}

void CameraSystem::SetViewRotation(CameraComponent& _camera, const glm::vec3& _position, const float _yaw, const float _pitch, const float _roll) const
{
	const float c3 = glm::cos(_roll);
	const float s3 = glm::sin(_roll);
	const float c2 = glm::cos(_pitch);
	const float s2 = glm::sin(_pitch);
	const float c1 = glm::cos(_yaw);
	const float s1 = glm::sin(_yaw);
	const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
	const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
	const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
	_camera.ViewMatrix = glm::mat4{ 1.f };
	_camera.ViewMatrix[0][0] = u.x;
	_camera.ViewMatrix[1][0] = u.y;
	_camera.ViewMatrix[2][0] = u.z;
	_camera.ViewMatrix[0][1] = v.x;
	_camera.ViewMatrix[1][1] = v.y;
	_camera.ViewMatrix[2][1] = v.z;
	_camera.ViewMatrix[0][2] = w.x;
	_camera.ViewMatrix[1][2] = w.y;
	_camera.ViewMatrix[2][2] = w.z;
	_camera.ViewMatrix[3][0] = -glm::dot(u, _position);
	_camera.ViewMatrix[3][1] = -glm::dot(v, _position);
	_camera.ViewMatrix[3][2] = -glm::dot(w, _position);
}

void CameraSystem::SetViewRotation(CameraComponent& _camera, const CameraTransformComponent& _transform) const
{
	SetViewRotation(_camera, _transform.Position, _transform.Rotation.y, _transform.Rotation.x, _transform.Rotation.z);
}


VESPERENGINE_NAMESPACE_END
