// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\ViewerApp.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "ViewerApp.h"

#include <array>
#include <stdexcept>
#include <iostream>
#include <chrono>

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/ext.hpp"

// ONLY IF USING VESPER ENGINE AS DLL, IF USING AS STATIC LIB, DON'T ADD!
#if defined(VESPERENGINE_DLL_IMPORT)
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#endif


#define DESCRIPTOR_MAX_SET_COUNT 100
#define DESCRIPTOR_SET_COUNT_PER_POOL 20


VESPERENGINE_USING_NAMESPACE


ViewerApp::ViewerApp(Config& _config) :
	VesperApp(_config)
{
	m_window = std::make_unique<ViewerWindow>(_config.WindowWidth, _config.WindowHeight, _config.WindowName);

	m_device = std::make_unique<Device>(*m_window);
	m_renderer = std::make_unique<Renderer>(*m_window, *m_device);

	m_globalPool = DescriptorPool::Builder(*m_device)
		.SetMaxSets(SwapChain::kMaxFramesInFlight * DESCRIPTOR_MAX_SET_COUNT)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::kMaxFramesInFlight * DESCRIPTOR_SET_COUNT_PER_POOL)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SwapChain::kMaxFramesInFlight * DESCRIPTOR_SET_COUNT_PER_POOL)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::kMaxFramesInFlight * DESCRIPTOR_SET_COUNT_PER_POOL)
		//.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SwapChain::kMaxFramesInFlight)
		.Build();

	m_entityHandlerSystem = std::make_unique<EntityHandlerSystem>(*this, *m_device);
	m_gameEntitySystem = std::make_unique<GameEntitySystem>(*this);
	m_materialSystem = std::make_unique<MaterialSystem>(*this, *m_device);
	m_modelSystem = std::make_unique<ModelSystem>(*this, *m_device, *m_materialSystem);

	m_masterRenderSystem = std::make_unique<MasterRenderSystem>(*m_device);

	m_opaqueRenderSystem = std::make_unique<OpaqueRenderSystem>(*this , *m_device, *m_globalPool,
		m_renderer->GetSwapChainRenderPass(),
		m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
		m_entityHandlerSystem->GetEntityDescriptorSetLayout());

	m_cameraSystem = std::make_unique<CameraSystem>(*this);
	m_objLoader = std::make_unique<ObjLoader>(*this , *m_device);

	LOG_NL();

	// BRDF LUT TEXTURE
	LOG(Logger::INFO, "Generating or loading BRDF LUT texture");
	const std::string brdfLutPath = GetConfig().TexturesPath + "brdf_lut.png";
	VkExtent2D extent;
	extent.width = 512;
	extent.height = 512;
	m_materialSystem->GenerateOrLoadBRDFLutTexture(brdfLutPath, extent);
	LOG(Logger::INFO, "BRDF LUT texture generated/loaded at ", brdfLutPath);



	//////////////////////////////////////////////////////////////////////////
	// Game side initialization

	m_keyboardController = std::make_unique<KeyboardMovementCameraController>(*this);

	m_mouseController = std::make_unique<MouseLookCameraController>();
	m_mouseController->SetMouseCallback(this, m_window->GetWindow());

	m_gameManager = std::make_unique<GameManager>(*this, *m_entityHandlerSystem, *m_gameEntitySystem, *m_modelSystem, *m_cameraSystem, *m_objLoader);

	m_gameManager->LoadCameraEntities();
	m_gameManager->LoadGameEntities();

	m_opaqueRenderSystem->MaterialBinding();
}

ViewerApp::~ViewerApp()
{
	m_gameManager->UnloadGameEntities();

	m_materialSystem->Cleanup();
	m_masterRenderSystem->Cleanup();
}

void ViewerApp::Run()
{	
	auto currentTime = std::chrono::high_resolution_clock::now();

	CameraComponent activeCameraComponent;
	CameraTransformComponent activeCameraTransformComponent;

	m_entityHandlerSystem->Initialize(*m_globalPool);
	m_masterRenderSystem->Initialize(*m_globalPool);

	while (!m_window->ShouldClose())
	{
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		const float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		// We could clamp frameTime to use a min between it and a max frame value, to avoid big "hiccups" if the renderer stop (i.e. resizing window while moving the camera)

		m_keyboardController->MoveInPlaneXZ(m_window->GetWindow(), frameTime);
		m_mouseController->Update(frameTime);

		auto commandBuffer = m_renderer->BeginFrame();
		if (commandBuffer != VK_NULL_HANDLE)
		{
			const int32 frameIndex = m_renderer->GetFrameIndex();

			FrameInfo frameInfo { frameIndex, frameTime, commandBuffer, 
				m_masterRenderSystem->GetGlobalDescriptorSet(frameIndex), m_entityHandlerSystem->GetEntityDescriptorSet(frameIndex) };
			
			m_gameManager->Update(frameInfo);

			m_opaqueRenderSystem->Update(frameInfo);

			const float aspectRatio = m_renderer->GetAspectRatio();
			m_cameraSystem->Update(aspectRatio);
			m_cameraSystem->GetActiveCameraData(0, activeCameraComponent, activeCameraTransformComponent);

			m_masterRenderSystem->UpdateScene(frameInfo, activeCameraComponent);
			m_entityHandlerSystem->UpdateEntities(frameInfo);

			// For instance, add here before the swap chain:
			// begin off screen shadow pass
			//	render shadow casting objects
			// end off screen shadow pass

			m_renderer->BeginSwapChainRenderPass(commandBuffer);

			m_masterRenderSystem->BindGlobalDescriptor(frameInfo);

			m_opaqueRenderSystem->Render(frameInfo);

			m_renderer->EndSwapChainRenderPass(commandBuffer);
			m_renderer->EndFrame();
		}
	}

	vkDeviceWaitIdle(m_device->GetDevice());
}
