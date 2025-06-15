// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\ViewerApp.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "WindowHandle.h"

#include "GameManager.h"
#include "KeyboardMovementCameraController.h"
#include "MouseLookCameraController.h"

#include <memory>
#include <vector>


VESPERENGINE_USING_NAMESPACE

class PhongCustomOpaqueRenderSystem;
class PhongCustomTransparentRenderSystem;

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
	// from engine side
	std::unique_ptr<ViewerWindow> m_window;
	std::unique_ptr<Device> m_device;
	std::unique_ptr<Renderer> m_renderer;

	std::unique_ptr<EntityHandlerSystem> m_entityHandlerSystem;
	std::unique_ptr<GameEntitySystem> m_gameEntitySystem;
	std::unique_ptr<ModelSystem> m_modelSystem;
	std::unique_ptr<TextureSystem> m_textureSystem;
	std::unique_ptr<MaterialSystem> m_materialSystem;
    std::unique_ptr<MasterRenderSystem> m_masterRenderSystem;
    
	// IN-ENGINE SYSTEMS
	std::unique_ptr<PhongOpaqueRenderSystem> m_phongOpaqueRenderSystem;
    std::unique_ptr<PhongTransparentRenderSystem> m_phongTransparentRenderSystem;
	std::unique_ptr<PBROpaqueRenderSystem> m_pbrOpaqueRenderSystem;
	std::unique_ptr<PBRTransparentRenderSystem> m_pbrTransparentRenderSystem;
	
	// CUSTOM IN-APP SYSTEMS
	//std::unique_ptr<PhongCustomOpaqueRenderSystem> m_phongOpaqueRenderSystem;
	//std::unique_ptr<PhongCustomTransparentRenderSystem> m_phongTransparentRenderSystem;

    std::unique_ptr<SkyboxRenderSystem> m_skyboxRenderSystem;

	std::unique_ptr<CameraSystem> m_cameraSystem;
	std::unique_ptr<ObjLoader> m_objLoader;
	std::unique_ptr<GltfLoader> m_gltfLoader;

	// from game side
	std::unique_ptr<KeyboardMovementCameraController> m_keyboardController;
	std::unique_ptr<MouseLookCameraController> m_mouseController;
	std::unique_ptr<GameManager> m_gameManager;
};

