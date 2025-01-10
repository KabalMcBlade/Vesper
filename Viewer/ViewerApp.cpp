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

// special struct to just inform the system this entity can be rotated
struct RotationComponent
{
	glm::vec3 RotationAxis;
	float RadiantPerFrame;
};

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

	m_keyboardController = std::make_unique<KeyboardMovementCameraController>(*this);

	m_mouseController = std::make_unique<MouseLookCameraController>();
	m_mouseController->SetMouseCallback(this, m_window->GetWindow());
	
	LOG_NL();

	// BRDF LUT TEXTURE
	LOG(Logger::INFO, "Generating or loading BRDF LUT texture");
	const std::string brdfLutPath = GetConfig().TexturesPath + "brdf_lut.png";
	VkExtent2D extent;
	extent.width = 512;
	extent.height = 512;
	m_materialSystem->GenerateOrLoadBRDFLutTexture(brdfLutPath, extent);
	LOG(Logger::INFO, "BRDF LUT texture generated/loaded at ", brdfLutPath);

	// test
	GetComponentManager().RegisterComponent<RotationComponent>(); 

	LoadCameraEntities();
	LoadGameEntities();
}

ViewerApp::~ViewerApp()
{
	// test
	GetComponentManager().UnregisterComponent<RotationComponent>();

	UnloadGameEntities();
}

void ViewerApp::Run()
{	
	auto currentTime = std::chrono::high_resolution_clock::now();

	CameraComponent activeCameraComponent;
	CameraTransformComponent activeCameraTransformComponent;

	m_entityHandlerSystem->Initialize(*m_globalPool);
	m_masterRenderSystem->Initialize(*m_globalPool);

	ecs::EntityManager& entityManager = GetEntityManager();
	ecs::ComponentManager& componentManager = GetComponentManager();

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
			
			const float aspectRatio = m_renderer->GetAspectRatio();

			//////////////////////////////////////////////////////////////////////////
			// ROTATION TEST
			for (auto gameEntity : ecs::IterateEntitiesWithAll<TransformComponent, RotationComponent>(entityManager, componentManager))
			{
				RotationComponent& rotateComponent = componentManager.GetComponent<RotationComponent>(gameEntity);
				TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);

				// Add random rotation, for testing UBO
				const glm::quat& prevRot = transformComponent.Rotation;
				glm::quat currRot = glm::angleAxis(rotateComponent.RadiantPerFrame, rotateComponent.RotationAxis);
				transformComponent.Rotation = prevRot * currRot;
			}
			//////////////////////////////////////////////////////////////////////////

			m_opaqueRenderSystem->Update(frameInfo);
			 
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

void ViewerApp::LoadCameraEntities()
{
	{
		ecs::Entity camera = m_gameEntitySystem->CreateGameEntity(EntityType::Camera);

		CameraTransformComponent& transformComponent = GetComponentManager().GetComponent<CameraTransformComponent>(camera);
		transformComponent.Position = { 0.0f, -0.5f, -3.5f };

		CameraComponent& cameraComponent = GetComponentManager().GetComponent<CameraComponent>(camera);

		m_cameraSystem->SetViewRotation(cameraComponent, transformComponent);
		m_cameraSystem->SetPerspectiveProjection(cameraComponent, glm::radians(50.0f), 1.0f, 0.1f, 100.0f);

		// Active this one by default
		m_cameraSystem->SetCurrentActiveCamera(camera);
	}
}

void ViewerApp::LoadGameEntities()
{
	//////////////////////////////////////////////////////////////////////////
	// Cube no Indices
	{
		std::unique_ptr<ModelData> cubeNoIndicesData = PrimitiveFactory::GenerateCubeNoIndices(
			{ 0.0f, 0.0f, 0.0f },
			{ glm::vec3(.9f, .9f, .9f), glm::vec3(.8f, .8f, .1f), glm::vec3(.9f, .6f, .1f), glm::vec3(.8f, .1f, .1f), glm::vec3(.1f, .1f, .8f), glm::vec3(.1f, .8f, .1f) }
		);

		ecs::Entity cubeNoIndices = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

		m_modelSystem->LoadModel(cubeNoIndices, std::move(cubeNoIndicesData));

		TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(cubeNoIndices);
		transformComponent.Position = { -1.0f, -1.0f, 0.0f };
		transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_entityHandlerSystem->RegisterEntity(cubeNoIndices);

		// test
		GetComponentManager().AddComponent<RotationComponent>(cubeNoIndices);

		static const float radPerFrame = 0.00174533f;     // 0.1 deg
		RotationComponent& rotateComponent = GetComponentManager().GetComponent<RotationComponent>(cubeNoIndices);
		rotateComponent.RotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		rotateComponent.RadiantPerFrame = radPerFrame;
	}


	//////////////////////////////////////////////////////////////////////////
	// Cube
	{
		std::vector<std::unique_ptr<ModelData>> coloredCubeDataList = m_objLoader->LoadModel("colored_cube.obj");
		for (auto& coloredCubeData : coloredCubeDataList)
		{
			ecs::Entity coloredCube = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);
			
			m_modelSystem->LoadModel(coloredCube, std::move(coloredCubeData));

			TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(coloredCube);
			transformComponent.Position = { 1.0f, -1.5f, 0.0f };
			transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem->RegisterEntity(coloredCube);

			// test
			GetComponentManager().AddComponent<RotationComponent>(coloredCube);

			static const float radPerFrame = 0.00174533f;     // 0.1 deg
			RotationComponent& rotateComponent = GetComponentManager().GetComponent<RotationComponent>(coloredCube);
			rotateComponent.RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
			rotateComponent.RadiantPerFrame = radPerFrame;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Flat Vase
	{
		std::vector<std::unique_ptr<ModelData>> flatVaseDataList = m_objLoader->LoadModel("flat_vase.obj");
		for (auto& flatVaseData : flatVaseDataList)
		{
			ecs::Entity flatVase = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

			m_modelSystem->LoadModel(flatVase, std::move(flatVaseData));

			TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(flatVase);
			transformComponent.Position = { -1.0f, 0.5f, 0.0f };
			transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem->RegisterEntity(flatVase);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Smooth Vase
	{
		std::vector<std::unique_ptr<ModelData>> smoothVaseDataList = m_objLoader->LoadModel("smooth_vase.obj");
		for (auto& smoothVaseData : smoothVaseDataList)
		{
			ecs::Entity smoothVase = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

			m_modelSystem->LoadModel(smoothVase, std::move(smoothVaseData));

			TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(smoothVase);
			transformComponent.Position = { 1.0f, 0.5f, 0.0f };
			transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem->RegisterEntity(smoothVase);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Plane / Quad
	{
		std::vector<std::unique_ptr<ModelData>> quadDataList = m_objLoader->LoadModel("quad.obj");
		for (auto& quadData : quadDataList)
		{
			ecs::Entity quad = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

			m_modelSystem->LoadModel(quad, std::move(quadData));

			TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(quad);
			transformComponent.Position = { 0.0f, 1.0f, 0.0f };
			transformComponent.Scale = { 3.0f, 1.0f, 3.0f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem->RegisterEntity(quad);
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	// A_blonde_twintailed_g_1228205950_texture
	{
		std::vector<std::unique_ptr<ModelData>> characterDataList = m_objLoader->LoadModel("A_blonde_twintailed_g_1228205950_texture.obj");
		for (auto& characterData : characterDataList)
		{
			ecs::Entity character = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

			m_modelSystem->LoadModel(character, std::move(characterData));

			TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(character);
			transformComponent.Position = { 0.0f, 0.0f, 1.0f };
			transformComponent.Scale = { 1.0f, 1.0f, 1.0f }; 
			transformComponent.Rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			m_entityHandlerSystem->RegisterEntity(character);
		}
	} 

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL BINDING FOR OPAQUE PIPELINE
	m_opaqueRenderSystem->MaterialBinding();
}

void ViewerApp::UnloadGameEntities()
{
	m_entityHandlerSystem->Cleanup();
	m_materialSystem->Cleanup();
	m_masterRenderSystem->Cleanup();
	m_modelSystem->UnloadModels();
	m_gameEntitySystem->DestroyGameEntities();
}