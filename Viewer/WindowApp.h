#pragma once

#include "WindowHandle.h"

#include "KeyboardMovementCameraController.h"
#include "MouseLookCameraController.h"

// test for fun:
#include "RainbowSystem.h"

#include <memory>
#include <vector>


VESPERENGINE_USING_NAMESPACE

class WindowApp final
{
public:
	WindowApp(Config& _config);
	~WindowApp();

	WindowApp(const WindowApp&) = delete;
	WindowApp& operator=(const WindowApp&) = delete;

public:
	void Run();

private:
	void LoadCameraEntities();
	void LoadGameEntities();
	void UnloadGameEntities();

private:
	// from engine side
	std::unique_ptr<ViewerWindow> m_window;
	std::unique_ptr<Device> m_device;
	std::unique_ptr<Renderer> m_renderer;

	std::unique_ptr<GameEntitySystem> m_gameEntitySystem;
	std::unique_ptr<ModelSystem> m_modelSystem;
	std::unique_ptr<SimpleRenderSystem> m_simpleRenderSystem;

	std::unique_ptr<CameraSystem> m_cameraSystem;

	// from game side
	std::unique_ptr<KeyboardMovementCameraController> m_keyboardController;
	std::unique_ptr<MouseLookCameraController> m_mouseController;

	// for fun
	std::unique_ptr<RainbowSystem> m_rainbowSystem;
};

