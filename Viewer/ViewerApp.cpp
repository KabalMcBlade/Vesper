// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\ViewerApp.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "ViewerApp.h"

#include "Systems/skybox_render_system.h"
#include "Systems/PhongCustomOpaqueRenderSystem.h"
#include "Systems/PhongCustomTransparentRenderSystem.h"


#include <array>
#include <stdexcept>
#include <iostream>
#include <chrono>


// ONLY IF USING VESPER ENGINE AS DLL, IF USING AS STATIC LIB, DON'T ADD!
#if defined(VESPERENGINE_DLL_IMPORT)
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#endif


#define DESCRIPTOR_MAX_SET_COUNT 512
#define DESCRIPTOR_MAX_COUNT_PER_POOL_TYPE 2048


VESPERENGINE_USING_NAMESPACE


ViewerApp::ViewerApp(Config& _config) :
	VesperApp(_config)
{
	m_window = std::make_unique<ViewerWindow>(_config.WindowWidth, _config.WindowHeight, _config.WindowName);

	m_device = std::make_unique<Device>(*m_window);
	m_renderer = std::make_unique<Renderer>(*m_window, *m_device);

	m_renderer->SetupGlobalDescriptors(
		{ { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DESCRIPTOR_MAX_COUNT_PER_POOL_TYPE }
		, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESCRIPTOR_MAX_COUNT_PER_POOL_TYPE}
		, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTOR_MAX_COUNT_PER_POOL_TYPE }
		, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DESCRIPTOR_MAX_COUNT_PER_POOL_TYPE}
		}
		, DESCRIPTOR_MAX_SET_COUNT);

	m_entityHandlerSystem = std::make_unique<EntityHandlerSystem>(*this, *m_device, *m_renderer);
	m_gameEntitySystem = std::make_unique<GameEntitySystem>(*this);
	m_textureSystem = std::make_unique<TextureSystem>(*this, *m_device);
	m_materialSystem = std::make_unique<MaterialSystem>(*m_device, *m_textureSystem);
	m_modelSystem = std::make_unique<ModelSystem>(*this, *m_device, *m_materialSystem);
	m_lightSystem = std::make_unique<LightSystem>(*this, *m_gameEntitySystem);
	m_blendShapeAnimationSystem = std::make_unique<BlendShapeAnimationSystem>(*this);

    m_masterRenderSystem = std::make_unique<MasterRenderSystem>(*m_device, *m_renderer, *m_lightSystem);
	
	// IN-ENGINE SYSTEMS
	// PHONG
    m_phongOpaqueRenderSystem = std::make_unique<PhongOpaqueRenderSystem>(*this , *m_device, *m_renderer,
            m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
            m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
            m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

	m_phongOpaqueRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

    m_phongTransparentRenderSystem = std::make_unique<PhongTransparentRenderSystem>(*this, *m_device, *m_renderer,
            m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
            m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
            m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

	m_phongTransparentRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

	// PBR
	m_pbrOpaqueRenderSystem = std::make_unique<PBROpaqueRenderSystem>(*this, *m_device, *m_renderer,
		m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
		m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
		m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

	m_pbrOpaqueRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

	m_pbrTransparentRenderSystem = std::make_unique<PBRTransparentRenderSystem>(*this, *m_device, *m_renderer,
		m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
		m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
		m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

	m_pbrTransparentRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

	// CUSTOM IN-APP SYSTEMS
	/*
	m_phongOpaqueRenderSystem = std::make_unique<PhongCustomOpaqueRenderSystem>(*this, *m_device, *m_renderer,
		m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
		m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
		m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

	m_phongOpaqueRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

    m_phongTransparentRenderSystem = std::make_unique<PhongCustomTransparentRenderSystem>(*this, *m_device, *m_renderer,
            m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
            m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
            m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

    m_phongTransparentRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());
	*/

    m_skyboxRenderSystem = std::make_unique<SkyboxRenderSystem>(*this, *m_device, *m_renderer,
            m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
            m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

    m_skyboxRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

	m_cameraSystem = std::make_unique<CameraSystem>(*this);
	m_objLoader = std::make_unique<ObjLoader>(*this , *m_device, *m_materialSystem);
	m_gltfLoader = std::make_unique<GltfLoader>(*this, *m_device, *m_materialSystem);

	LOG_NL();

	//////////////////////////////////////////////////////////////////////////
	// Game side initialization

	m_keyboardController = std::make_unique<KeyboardMovementCameraController>(*this);

	m_mouseController = std::make_unique<MouseLookCameraController>();
	m_mouseController->SetMouseCallback(this, m_window->GetWindow());
	MouseLookCameraController::SetKeyboardController(m_keyboardController.get());

	m_gameManager = std::make_unique<GameManager>(
		*this, 
		*m_entityHandlerSystem, 
		*m_gameEntitySystem, 
		*m_modelSystem, 
		*m_materialSystem,
		*m_cameraSystem, 
		*m_objLoader,
		*m_gltfLoader,
		*m_textureSystem,
		*m_lightSystem,
		*m_blendShapeAnimationSystem);

    m_gameManager->LoadCameraEntities();
    m_gameManager->LoadGameEntities();
	m_gameManager->LoadLights();

    m_phongOpaqueRenderSystem->MaterialBinding();
    m_phongTransparentRenderSystem->MaterialBinding();
	m_pbrOpaqueRenderSystem->MaterialBinding();
	m_pbrTransparentRenderSystem->MaterialBinding();
    m_skyboxRenderSystem->MaterialBinding();
}

ViewerApp::~ViewerApp()
{
	m_gameManager->UnloadGameEntities();
	m_textureSystem->Cleanup();
    m_materialSystem->Cleanup();
    m_phongOpaqueRenderSystem->Cleanup();
    m_phongTransparentRenderSystem->Cleanup();
	m_pbrOpaqueRenderSystem->Cleanup();
	m_pbrTransparentRenderSystem->Cleanup();
    m_skyboxRenderSystem->Cleanup();
    m_masterRenderSystem->Cleanup();
}

void ViewerApp::Run()
{	
	auto currentTime = std::chrono::high_resolution_clock::now();

	CameraComponent activeCameraComponent;
	CameraTransformComponent activeCameraTransformComponent;

	m_entityHandlerSystem->Initialize();
	m_masterRenderSystem->Initialize(*m_textureSystem, *m_materialSystem,
		m_gameManager->GetIrradianceMap(),
		m_gameManager->GetPrefilteredEnvMap(),
		m_gameManager->GetBrdfLut());

	while (!m_window->ShouldClose())
	{
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		const float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		// We could clamp frameTime to use a min between it and a max frame value, to avoid big "hiccups" if the renderer stop (i.e. resizing window while moving the camera)

		m_keyboardController->MoveInPlaneXZ(m_window->GetWindow(), frameTime);
		m_keyboardController->SetNextAnimation(m_blendShapeAnimationSystem.get());
		m_mouseController->Update(frameTime);

		auto commandBuffer = m_renderer->BeginFrame();
		if (commandBuffer != VK_NULL_HANDLE)
		{
			const int32 frameIndex = m_renderer->GetFrameIndex();

            FrameInfo frameInfo { frameIndex, frameTime, commandBuffer,
                    m_masterRenderSystem->GetGlobalDescriptorSet(frameIndex),
                    m_entityHandlerSystem->GetEntityDescriptorSet(frameIndex),
					m_masterRenderSystem->GetBindlessBindingDescriptorSet(frameIndex) };
			
			m_gameManager->Update(frameInfo);

            m_phongOpaqueRenderSystem->Update(frameInfo);
            m_phongTransparentRenderSystem->Update(frameInfo);

			m_pbrOpaqueRenderSystem->Update(frameInfo);
			m_pbrTransparentRenderSystem->Update(frameInfo);

            m_skyboxRenderSystem->Update(frameInfo);

			m_blendShapeAnimationSystem->Update(frameInfo);

			const float aspectRatio = m_renderer->GetAspectRatio();
			m_cameraSystem->Update(aspectRatio);
			m_cameraSystem->GetActiveCameraData(0, activeCameraComponent, activeCameraTransformComponent);

			m_masterRenderSystem->UpdateScene(frameInfo, activeCameraComponent, activeCameraTransformComponent);
			m_entityHandlerSystem->UpdateEntities(frameInfo);

			// For instance, add here before the swap chain:
			// begin off screen shadow pass
			//	render shadow casting objects
			// end off screen shadow pass
			
			m_masterRenderSystem->BindGlobalDescriptor(frameInfo);

            m_renderer->BeginSwapChainRenderPass(commandBuffer);

            m_skyboxRenderSystem->Render(frameInfo);

            m_phongOpaqueRenderSystem->Render(frameInfo);
			m_phongTransparentRenderSystem->Render(frameInfo);

			m_pbrOpaqueRenderSystem->Render(frameInfo);
			m_pbrTransparentRenderSystem->Render(frameInfo);

			m_renderer->EndSwapChainRenderPass(commandBuffer);
			m_renderer->EndFrame();
		}
	}

	vkDeviceWaitIdle(m_device->GetDevice());
}
