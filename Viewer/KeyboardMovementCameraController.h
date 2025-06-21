// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\KeyboardMovementCameraController.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#define GLFW_INCLUDE_VULKAN
#include "ThirdParty/glfw/include/glfw3.h"

#include "vesper.h"


VESPERENGINE_USING_NAMESPACE

class KeyboardMovementCameraController
{
public:
	KeyboardMovementCameraController(VesperApp& _app,
		BlendShapeAnimationSystem& _blendShapeAnimationSystem);
	~KeyboardMovementCameraController() = default;

public:
	struct KeyMappings 
	{
		int32 MoveLeft = GLFW_KEY_A;
		int32 MoveRight = GLFW_KEY_D;
		int32 MoveForward = GLFW_KEY_W;
		int32 MoveBackward = GLFW_KEY_S;
		int32 MoveUp = GLFW_KEY_E;
		int32 MoveDown = GLFW_KEY_Q;
		int32 LookLeft = GLFW_KEY_LEFT;
		int32 LookRight = GLFW_KEY_RIGHT;
		int32 LookUp = GLFW_KEY_UP;
		int32 LookDown = GLFW_KEY_DOWN;
		int32 LookRollRight = GLFW_KEY_PAGE_UP;
		int32 LookRollLeft = GLFW_KEY_PAGE_DOWN;
		int32 ToggleLights = GLFW_KEY_L;
		int32 NextAnimation = GLFW_KEY_N;
	};

	void Update(GLFWwindow* _window, float _dt);

private:
	bool IsKeyJustPressed(int32 _key, GLFWwindow* _window);

private:
	VesperApp& m_app;
	BlendShapeAnimationSystem& m_blendShapeAnimationSystem;

	KeyMappings m_keys{};
	std::unordered_map<int32, bool> m_previousKeyState;

	friend class MouseLookCameraController;
	float m_moveSpeed{3.0f};
	float m_lookSpeed{3.0f};

	bool m_limitLook{true};
	bool m_showLights{ false };
};

