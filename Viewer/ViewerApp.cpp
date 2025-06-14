// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\ViewerApp.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "ViewerApp.h"

#include "Systems/skybox_render_system.h"
#include "Systems/CustomOpaqueRenderSystem.h"
#include "Systems/CustomTransparentRenderSystem.h"


#include <array>
#include <stdexcept>
#include <iostream>
#include <chrono>


// ONLY IF USING VESPER ENGINE AS DLL, IF USING AS STATIC LIB, DON'T ADD!
#if defined(VESPERENGINE_DLL_IMPORT)
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#endif


#define DESCRIPTOR_MAX_SET_COUNT 32
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
	m_texturelSystem = std::make_unique<TextureSystem>(*this, *m_device);
	m_materialSystem = std::make_unique<MaterialSystem>(*m_device, *m_texturelSystem);
	m_modelSystem = std::make_unique<ModelSystem>(*this, *m_device, *m_materialSystem);

    m_masterRenderSystem = std::make_unique<MasterRenderSystem>(*m_device, *m_renderer);

	
	// IN-ENGINE SYSTEMS
	/*
    m_opaqueRenderSystem = std::make_unique<OpaqueRenderSystem>(*this , *m_device, *m_renderer,
            m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
            m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
            m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

	m_opaqueRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

    m_transparentRenderSystem = std::make_unique<TransparentRenderSystem>(*this, *m_device, *m_renderer,
            m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
            m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
            m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

	m_transparentRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());
	*/

	// CUSTOM IN-APP SYSTEMS
	m_opaqueRenderSystem = std::make_unique<CustomOpaqueRenderSystem>(*this, *m_device, *m_renderer,
		m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
		m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
		m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

	m_opaqueRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

        m_transparentRenderSystem = std::make_unique<CustomTransparentRenderSystem>(*this, *m_device, *m_renderer,
                m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
                m_entityHandlerSystem->GetEntityDescriptorSetLayout(),
                m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

        m_transparentRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

        m_skyboxRenderSystem = std::make_unique<SkyboxRenderSystem>(*this, *m_device, *m_renderer,
                m_masterRenderSystem->GetGlobalDescriptorSetLayout(),
                m_masterRenderSystem->GetBindlessBindingDescriptorSetLayout());

        m_skyboxRenderSystem->CreatePipeline(m_renderer->GetSwapChainRenderPass());

	m_cameraSystem = std::make_unique<CameraSystem>(*this);
	m_objLoader = std::make_unique<ObjLoader>(*this , *m_device, *m_materialSystem);

	LOG_NL();

	// BRDF LUT TEXTURE
	LOG(Logger::INFO, "Generating or loading BRDF LUT texture");
	const std::string brdfLutPath = GetConfig().TexturesPath + "brdf_lut.png";
	VkExtent2D extent;
	extent.width = 512;
	extent.height = 512;
	std::shared_ptr<TextureData> brdfLut = m_texturelSystem->GenerateOrLoadBRDFLutTexture(brdfLutPath, extent);
	LOG(Logger::INFO, "BRDF LUT texture generated/loaded at ", brdfLutPath);

	LOG_NL();

	// CUBEMAP TEXTURE TEST
	const std::string cubemapTexturesDirectoryPath = GetConfig().TexturesPath + "Yokohama3_CubeMap/";
	LOG(Logger::INFO, "Loading Cubemap texture: ", cubemapTexturesDirectoryPath);

	std::array<std::string, 6> cubemapTexturesDirectoryFilepaths;
	cubemapTexturesDirectoryFilepaths[0] = cubemapTexturesDirectoryPath + "negx.jpg";
	cubemapTexturesDirectoryFilepaths[1] = cubemapTexturesDirectoryPath + "negy.jpg";
	cubemapTexturesDirectoryFilepaths[2] = cubemapTexturesDirectoryPath + "negz.jpg";
	cubemapTexturesDirectoryFilepaths[3] = cubemapTexturesDirectoryPath + "posx.jpg";
	cubemapTexturesDirectoryFilepaths[4] = cubemapTexturesDirectoryPath + "posy.jpg";
	cubemapTexturesDirectoryFilepaths[5] = cubemapTexturesDirectoryPath + "posz.jpg";

	std::shared_ptr<TextureData> cubeMap = m_texturelSystem->LoadCubemap(cubemapTexturesDirectoryFilepaths);
	LOG(Logger::INFO, "Cubemap loaded!");

	// CUBEMAP HDR TEXTURE TEST
	const std::string cubemapHdrTexturesPath = GetConfig().TexturesPath + "misty_pines_4k.hdr";
	LOG(Logger::INFO, "Loading Cubemap texture: ", cubemapHdrTexturesPath);

	std::shared_ptr<TextureData> cubeMapHdr = m_texturelSystem->LoadCubemap(cubemapHdrTexturesPath);
	LOG(Logger::INFO, "Cubemap HDR loaded!");

	LOG_NL();

	//////////////////////////////////////////////////////////////////////////
	// Game side initialization

	m_keyboardController = std::make_unique<KeyboardMovementCameraController>(*this);

	m_mouseController = std::make_unique<MouseLookCameraController>();
	m_mouseController->SetMouseCallback(this, m_window->GetWindow());

	m_gameManager = std::make_unique<GameManager>(
		*this, 
		*m_entityHandlerSystem, 
		*m_gameEntitySystem, 
		*m_modelSystem, 
		*m_materialSystem,
		*m_cameraSystem, 
		*m_objLoader);

    m_gameManager->LoadCameraEntities();
    m_gameManager->LoadGameEntities(cubeMap);

    m_opaqueRenderSystem->MaterialBinding();
    m_transparentRenderSystem->MaterialBinding();
    m_skyboxRenderSystem->MaterialBinding();
}

ViewerApp::~ViewerApp()
{
	m_gameManager->UnloadGameEntities();
	m_texturelSystem->Cleanup();
    m_materialSystem->Cleanup();
    m_opaqueRenderSystem->Cleanup();
    m_transparentRenderSystem->Cleanup();
    m_skyboxRenderSystem->Cleanup();
    m_masterRenderSystem->Cleanup();
}

void ViewerApp::Run()
{	
	auto currentTime = std::chrono::high_resolution_clock::now();

	CameraComponent activeCameraComponent;
	CameraTransformComponent activeCameraTransformComponent;

	m_entityHandlerSystem->Initialize();
	m_masterRenderSystem->Initialize(*m_texturelSystem, *m_materialSystem);

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
                    m_masterRenderSystem->GetGlobalDescriptorSet(frameIndex),
                    m_entityHandlerSystem->GetEntityDescriptorSet(frameIndex),
					m_masterRenderSystem->GetBindlessBindingDescriptorSet(frameIndex) };
			
			m_gameManager->Update(frameInfo);

            m_opaqueRenderSystem->Update(frameInfo);
            m_transparentRenderSystem->Update(frameInfo);
            m_skyboxRenderSystem->Update(frameInfo);

			const float aspectRatio = m_renderer->GetAspectRatio();
			m_cameraSystem->Update(aspectRatio);
			m_cameraSystem->GetActiveCameraData(0, activeCameraComponent, activeCameraTransformComponent);

			m_masterRenderSystem->UpdateScene(frameInfo, activeCameraComponent);
			m_entityHandlerSystem->UpdateEntities(frameInfo);

			// For instance, add here before the swap chain:
			// begin off screen shadow pass
			//	render shadow casting objects
			// end off screen shadow pass

			m_masterRenderSystem->BindGlobalDescriptor(frameInfo);

            m_renderer->BeginSwapChainRenderPass(commandBuffer);

            m_skyboxRenderSystem->Render(frameInfo);
            m_opaqueRenderSystem->Render(frameInfo);
            m_transparentRenderSystem->Render(frameInfo);

			m_renderer->EndSwapChainRenderPass(commandBuffer);
			m_renderer->EndFrame();
		}
	}

	vkDeviceWaitIdle(m_device->GetDevice());
}
