// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\MouseLookCameraController.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "MouseLookCameraController.h"
#include "KeyboardMovementCameraController.h"


VESPERENGINE_USING_NAMESPACE

VesperApp* MouseLookCameraController::m_staticApp = nullptr;
KeyboardMovementCameraController* MouseLookCameraController::m_keyboardController = nullptr;

MouseLookCameraController& MouseLookCameraController::GetInstance()
{
	static MouseLookCameraController instance;
	return instance;
}

void MouseLookCameraController::MouseMoveCallback(GLFWwindow* _window, double _xPos, double _yPos)
{
	GetInstance().MouseMoveCallbackImpl(_window, _xPos, _yPos);
}

void MouseLookCameraController::MouseButtonCallback(GLFWwindow* _window, int32 _button, int32 _action, int32 _mods)
{
	GetInstance().MouseButtonCallbackImpl(_window,  _button, _action, _mods);
}

void MouseLookCameraController::MouseScrollCallback(GLFWwindow* _window, double _xOffset, double _yOffset)
{
	GetInstance().MouseScrollCallbackImpl(_window, _xOffset, _yOffset);
}

void MouseLookCameraController::SetMouseCallback(VesperApp* _app, GLFWwindow* _window)
{
	m_staticApp = _app;
	glfwSetCursorPosCallback(_window, &MouseLookCameraController::MouseMoveCallback);
	glfwSetMouseButtonCallback(_window, &MouseLookCameraController::MouseButtonCallback);
	glfwSetScrollCallback(_window, &MouseLookCameraController::MouseScrollCallback);
}

void MouseLookCameraController::SetKeyboardController(KeyboardMovementCameraController* controller)
{
	m_keyboardController = controller;
}

void MouseLookCameraController::Update(float _dt)
{
	m_dt = _dt;
}

void MouseLookCameraController::MouseMoveCallbackImpl(GLFWwindow* _window, double _xPos, double _yPos)
{
	if (m_leftClickMousePressed)
	{
		const float xpos = static_cast<float>(_xPos);
		const float ypos = static_cast<float>(_yPos);

		float xoffset = xpos - m_lastX;
		float yoffset = m_lastY - ypos;
		m_lastX = xpos;
		m_lastY = ypos;

		xoffset *= m_dt * m_mouseSensitivity;
		yoffset *= m_dt * m_mouseSensitivity;

		ecs::EntityManager& entityManager = m_staticApp->GetEntityManager();
		ecs::ComponentManager& componentManager = m_staticApp->GetComponentManager();

		for (auto camera : ecs::IterateEntitiesWithAll<CameraActive, CameraTransformComponent>(entityManager, componentManager))
		{
			CameraTransformComponent& transformComponent = componentManager.GetComponent<CameraTransformComponent>(camera);

			transformComponent.Rotation.y += xoffset;
			transformComponent.Rotation.x += yoffset;
		}
	}
}

void MouseLookCameraController::MouseButtonCallbackImpl(GLFWwindow* _window, int32 _button, int32 _action, int32 _mods)
{
	if (_button == GLFW_MOUSE_BUTTON_LEFT)
	{
		m_leftClickMousePressed = _action == GLFW_PRESS;

		double xposDouble, yposDouble;
		glfwGetCursorPos(_window, &xposDouble, &yposDouble);

		const float xpos = static_cast<float>(xposDouble);
		const float ypos = static_cast<float>(yposDouble);

		if (m_firstClick)
		{
			m_lastX = xpos;
			m_lastY = ypos;
			m_firstClick = false;
		}
		else
		{
			float xoffset = xpos - m_lastX;
			float yoffset = m_lastY - ypos;
			m_lastX = xpos;
			m_lastY = ypos;
		}
	}
}

void MouseLookCameraController::MouseScrollCallbackImpl(GLFWwindow* _window, double _xOffset, double _yOffset)
{
	if (!m_keyboardController)
	{
		return;
	}

	const float speedStep = 0.2f;
	const float minSpeed = 0.2f;
	const float maxSpeed = 200.0f;

	if (_yOffset > 0.0)
	{
		m_keyboardController->m_moveSpeed += speedStep;
		m_keyboardController->m_lookSpeed += speedStep * 0.2f;
	}
	else if (_yOffset < 0.0)
	{
		m_keyboardController->m_moveSpeed -= speedStep;
		m_keyboardController->m_lookSpeed -= speedStep * 0.2f;
	}

	m_keyboardController->m_moveSpeed = std::clamp(m_keyboardController->m_moveSpeed, minSpeed, maxSpeed);
	m_keyboardController->m_lookSpeed = std::clamp(m_keyboardController->m_lookSpeed, minSpeed * 0.2f, maxSpeed * 0.2f);

}