// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\MouseLookCameraController.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#define GLFW_INCLUDE_VULKAN
#include "ThirdParty/glfw/include/glfw3.h"

#include "vesper.h"


VESPERENGINE_USING_NAMESPACE

class KeyboardMovementCameraController;

class MouseLookCameraController
{
public:
	MouseLookCameraController() = default;
	~MouseLookCameraController() = default;

public:
	static MouseLookCameraController& GetInstance();
	static void MouseMoveCallback(GLFWwindow* _window, double _xPos, double _yPos);
	static void MouseButtonCallback(GLFWwindow* _window, int32 _button, int32 _action, int32 _mods);
	static void MouseScrollCallback(GLFWwindow* _window, double _xOffset, double _yOffset);

	static void SetKeyboardController(KeyboardMovementCameraController* controller);

public:
	void SetMouseCallback(VesperApp* _app, GLFWwindow* _window);

	void Update(float _dt);

private:
	void MouseMoveCallbackImpl(GLFWwindow* _window, double _xPos, double _yPos);
	void MouseButtonCallbackImpl(GLFWwindow* _window, int32 _button, int32 _action, int32 _mods);
	void MouseScrollCallbackImpl(GLFWwindow* _window, double _xOffset, double _yOffset);

private:
	//VesperApp* m_app;
	static VesperApp* m_staticApp;
	static KeyboardMovementCameraController* m_keyboardController;

	float m_lastX{ 0.0f };
	float m_lastY{ 0.0f };
	float m_dt{1.0f/60.0f};
	float m_mouseSensitivity{ 0.1f };

	bool m_leftClickMousePressed{false};
	bool m_firstClick{true};
};

