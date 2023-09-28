#include "MouseLookCameraController.h"

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


VESPERENGINE_USING_NAMESPACE

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

void MouseLookCameraController::SetMouseCallback(GLFWwindow* _window)
{
	glfwSetCursorPosCallback(_window, &MouseLookCameraController::MouseMoveCallback);
	glfwSetMouseButtonCallback(_window, &MouseLookCameraController::MouseButtonCallback);
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

		for (auto camera : ecs::IterateEntitiesWithAll<CameraActive, CameraTransformComponent>())
		{
			CameraTransformComponent& transformComponent = ecs::ComponentManager::GetComponent<CameraTransformComponent>(camera);

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