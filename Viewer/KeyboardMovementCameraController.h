#pragma once

#define GLFW_INCLUDE_VULKAN
#include "ThirdParty/glfw/include/glfw3.h"

#include "vesper.h"


VESPERENGINE_USING_NAMESPACE

class KeyboardMovementCameraController
{
public:
	KeyboardMovementCameraController(VesperApp& _app);
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
	};

	void MoveInPlaneXZ(GLFWwindow* _window, float _dt);

private:
	VesperApp& m_app;
	KeyMappings m_keys{};

	float m_moveSpeed{3.0f};
	float m_lookSpeed{3.0f};

	bool m_limitLook{true};
};

