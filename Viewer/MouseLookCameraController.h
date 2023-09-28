#pragma once

#define GLFW_INCLUDE_VULKAN
#include "ThirdParty/glfw/include/glfw3.h"

#include "vesper.h"


VESPERENGINE_USING_NAMESPACE

class MouseLookCameraController
{
public:
	MouseLookCameraController() = default;
	~MouseLookCameraController() = default;

public:
	static MouseLookCameraController& GetInstance();
	static void MouseMoveCallback(GLFWwindow* _window, double _xPos, double _yPos);
	static void MouseButtonCallback(GLFWwindow* _window, int32 _button, int32 _action, int32 _mods);

public:
	void SetMouseCallback(GLFWwindow* _window);

	void Update(float _dt);

private:
	void MouseMoveCallbackImpl(GLFWwindow* _window, double _xPos, double _yPos);
	void MouseButtonCallbackImpl(GLFWwindow* _window, int32 _button, int32 _action, int32 _mods);

private:
	float m_lastX{ 0.0f };
	float m_lastY{ 0.0f };
	float m_dt{1.0f/60.0f};
	float m_mouseSensitivity{ 0.1f };

	bool m_leftClickMousePressed{false};
	bool m_firstClick{true};
};
