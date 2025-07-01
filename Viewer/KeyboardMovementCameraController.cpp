// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\KeyboardMovementCameraController.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "KeyboardMovementCameraController.h"


VESPERENGINE_USING_NAMESPACE


KeyboardMovementCameraController::KeyboardMovementCameraController(VesperApp& _app,
	BlendShapeAnimationSystem& _blendShapeAnimationSystem)
	: m_app(_app)
	, m_blendShapeAnimationSystem(_blendShapeAnimationSystem)
{
}

bool KeyboardMovementCameraController::IsKeyJustPressed(int32 _key, GLFWwindow* _window)
{
	bool isPressedNow = glfwGetKey(_window, _key) == GLFW_PRESS;
	bool wasPressedBefore = m_previousKeyState[_key];

	m_previousKeyState[_key] = isPressedNow; // update for next frame

	return isPressedNow && !wasPressedBefore; // true only on the frame the key is first pressed
}

void KeyboardMovementCameraController::Update(GLFWwindow* _window, float _dt)
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	if (IsKeyJustPressed(m_keys.NextAnimation, _window))
	{
		m_blendShapeAnimationSystem.SetNextAnimationForAllEntities();
	}

	if (IsKeyJustPressed(m_keys.ToggleLights, _window))
    {
        m_showLights = !m_showLights;

        if (m_showLights)
        {
            for (auto entity : ecs::IterateEntitiesWithAny<DirectionalLightComponent, PointLightComponent, SpotLightComponent>(entityManager, componentManager))
            {
                if (!componentManager.HasComponents<VisibilityComponent>(entity))
                {
                    componentManager.AddComponent<VisibilityComponent>(entity);
                }
            }
        }
        else
        {
            for (auto entity : ecs::IterateEntitiesWithAny<DirectionalLightComponent, PointLightComponent, SpotLightComponent>(entityManager, componentManager))
            {
                if (componentManager.HasComponents<VisibilityComponent>(entity))
                {
                    componentManager.RemoveComponent<VisibilityComponent>(entity);
                }
            }
        }
    }

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
		const glm::vec3 upDir{ 0.f, 1.f, 0.f };

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
