// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\KeyboardMovementCameraController.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "KeyboardMovementCameraController.h"

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "gtx/quaternion.hpp"
#include "gtx/euler_angles.hpp"

VESPERENGINE_USING_NAMESPACE

KeyboardMovementCameraController::KeyboardMovementCameraController(VesperApp& _app)
	: m_app(_app)
{
}

void KeyboardMovementCameraController::MoveInPlaneXZ(GLFWwindow* _window, float _dt)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto camera : ecs::IterateEntitiesWithAll<CameraActive, CameraTransformComponent>(entityManager, componentManager))
	{
		CameraTransformComponent& transformComponent = componentManager.GetComponent<CameraTransformComponent>(camera);

		glm::vec3 rotate{ 0.0f };
		if (glfwGetKey(_window, m_keys.LookRight) == GLFW_PRESS) rotate.y += 1.0f;
		if (glfwGetKey(_window, m_keys.LookLeft) == GLFW_PRESS) rotate.y -= 1.0f;
		if (glfwGetKey(_window, m_keys.LookUp) == GLFW_PRESS) rotate.x += 1.0f;
		if (glfwGetKey(_window, m_keys.LookDown) == GLFW_PRESS) rotate.x -= 1.0f;
		if (glfwGetKey(_window, m_keys.LookRollRight) == GLFW_PRESS) rotate.z += 1.0f;
		if (glfwGetKey(_window, m_keys.LookRollLeft) == GLFW_PRESS) rotate.z -= 1.0f;

		if (glm::length(rotate) > std::numeric_limits<float>::epsilon())
		{
			transformComponent.Rotation += m_lookSpeed * _dt * glm::normalize(rotate);
		}

		if (m_limitLook)
		{
			// limit pitch values between about +/- 85ish degrees
			transformComponent.Rotation.x = glm::clamp(transformComponent.Rotation.x, -1.5f, 1.5f);
			transformComponent.Rotation.y = glm::mod(transformComponent.Rotation.y, glm::two_pi<float>());
		}

		float yaw = transformComponent.Rotation.y;
		const glm::vec3 forwardDir{ glm::sin(yaw), 0.f, glm::cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };
		if (glfwGetKey(_window, m_keys.MoveForward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(_window, m_keys.MoveBackward) == GLFW_PRESS) moveDir -= forwardDir;
		if (glfwGetKey(_window, m_keys.MoveRight) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(_window, m_keys.MoveLeft) == GLFW_PRESS) moveDir -= rightDir;
		if (glfwGetKey(_window, m_keys.MoveUp) == GLFW_PRESS) moveDir += upDir;
		if (glfwGetKey(_window, m_keys.MoveDown) == GLFW_PRESS) moveDir -= upDir;

		if (glm::length(moveDir) > std::numeric_limits<float>::epsilon())
		{
			transformComponent.Position += m_moveSpeed * _dt * glm::normalize(moveDir);
		}
	}
}
