#pragma once

#include "WindowHandle.h"

#include "KeyboardMovementCameraController.h"
#include "MouseLookCameraController.h"

#include <memory>
#include <vector>


VESPERENGINE_USING_NAMESPACE

class ViewerApp final : public VesperApp
{
public:
	ViewerApp(Config& _config);
	~ViewerApp();

	ViewerApp(const ViewerApp&) = delete;
	ViewerApp& operator=(const ViewerApp&) = delete;

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

	// this goes AFTER the device, or will crash at shutdown time.
	// also, a DescriptorPool could be x system, instead one global.
	std::unique_ptr<DescriptorPool> m_globalPool;	
	std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;

	std::unique_ptr<GameEntitySystem> m_gameEntitySystem;
	std::unique_ptr<ModelSystem> m_modelSystem;
	std::unique_ptr<SimpleRenderSystem> m_simpleRenderSystem;

	std::unique_ptr<CameraSystem> m_cameraSystem;
	std::unique_ptr<ObjLoader> m_objLoader;
	std::unique_ptr<Buffer> m_buffer;

	// from game side
	std::unique_ptr<KeyboardMovementCameraController> m_keyboardController;
	std::unique_ptr<MouseLookCameraController> m_mouseController;
};

