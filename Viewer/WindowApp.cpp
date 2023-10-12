#include "WindowApp.h"

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
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>


VESPERENGINE_USING_NAMESPACE

WindowApp::WindowApp(Config& _config) :
	VesperApp(_config)
{
	m_window = std::make_unique<ViewerWindow>(_config.WindowWidth, _config.WindowHeight, _config.WindowName);

	m_device = std::make_unique<Device>(*m_window);
	m_renderer = std::make_unique<Renderer>(*m_window, *m_device);

	// This should be managed as global pool per render system
	m_globalPool = DescriptorPool::Builder(*m_device)
		.SetMaxSets(SwapChain::kMaxFramesInFlight)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::kMaxFramesInFlight)
		.Build();

	// This should be part of the highest render system, which is shared among all the possible shaders and so all the possible Render systems
	m_globalSetLayout = DescriptorSetLayout::Builder(*m_device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build();

	m_gameEntitySystem = std::make_unique<GameEntitySystem>();
	m_modelSystem = std::make_unique<ModelSystem>(*m_device);
	m_simpleRenderSystem = std::make_unique<SimpleRenderSystem>(*m_device, m_renderer->GetSwapChainRenderPass(), m_globalSetLayout->GetDescriptorSetLayout());
	m_cameraSystem = std::make_unique<CameraSystem>();
	m_objLoader = std::make_unique<ObjLoader>(*m_device);
	m_buffer = std::make_unique<Buffer>(*m_device);

	m_keyboardController = std::make_unique<KeyboardMovementCameraController>();

	m_mouseController = std::make_unique<MouseLookCameraController>();
	m_mouseController->SetMouseCallback(m_window->GetWindow());

	m_rainbowSystem = std::make_unique<RainbowSystem>(60.0f);

	LoadCameraEntities();
	LoadGameEntities();
}

WindowApp::~WindowApp()
{
	UnloadGameEntities();
}

void WindowApp::Run()
{
	// This should be in separated class which should manage the "actual" game in terms of what render
// 	BufferComponent globalUBO;
// 	m_buffer->Create(
// 		globalUBO,
// 		sizeof(GlobalUBO),
// 		SwapChain::kMaxFramesInFlight,
// 		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
// 		VMA_MEMORY_USAGE_AUTO, //VMA_MEMORY_USAGE_AUTO,//VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
// 		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
// 		m_device->GetProperties().limits.minUniformBufferOffsetAlignment,
// 		true
// 	);

	std::vector<BufferComponent> uboBuffers(SwapChain::kMaxFramesInFlight);
	for (int i = 0; i < uboBuffers.size(); i++) 
	{
		m_buffer->Create(
			uboBuffers[i],
			sizeof(GlobalUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, //VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, //VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			1,
			true
		);
	}

// 	auto globalSetLayout = DescriptorSetLayout::Builder(*m_device)
// 		.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
// 		.Build();

	std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::kMaxFramesInFlight);
	for (int i = 0; i < globalDescriptorSets.size(); ++i)
	{
		auto bufferInfo = m_buffer->GetDescriptorInfo(uboBuffers[i]);
		DescriptorWriter(*m_globalSetLayout, *m_globalPool)
			.WriteBuffer(0, &bufferInfo)
			.Build(globalDescriptorSets[i]);
	}

	auto currentTime = std::chrono::high_resolution_clock::now();

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

			FrameInfo frameInfo { frameIndex, frameTime, commandBuffer, globalDescriptorSets[frameIndex] };
		        
			//camera
			const float aspectRatio = m_renderer->GetAspectRatio();
			for (auto camera : ecs::IterateEntitiesWithAll<CameraComponent, CameraTransformComponent, GlobalUBO>())
			{
				CameraTransformComponent& transformComponent = ecs::ComponentManager::GetComponent<CameraTransformComponent>(camera);
				CameraComponent& cameraComponent = ecs::ComponentManager::GetComponent<CameraComponent>(camera);

				m_cameraSystem->SetViewRotation(cameraComponent, transformComponent);
				m_cameraSystem->SetPerspectiveProjection(cameraComponent, glm::radians(50.0f), aspectRatio, 0.1f, 10.0f);

				GlobalUBO& ubo = ecs::ComponentManager::GetComponent<GlobalUBO>(camera);
				ubo.ProjectionView = cameraComponent.ProjectionMatrix * cameraComponent.ViewMatrix;

				//m_buffer->WriteToIndex(uboBuffers[frameIndex], &ubo, frameIndex);
				//m_buffer->FlushIndex(uboBuffers[frameIndex], frameIndex);
				m_buffer->WriteToBuffer(uboBuffers[frameIndex], &ubo);
				m_buffer->Flush(uboBuffers[frameIndex]);
			}


			// For instance, add here before the swap chain:
			// begin off screen shadow pass
			//	render shadow casting objects
			// end off screen shadow pass


			m_renderer->BeginSwapChainRenderPass(commandBuffer);
			//m_rainbowSystem->Update(1.0f/6.0f);	// for fun

			m_simpleRenderSystem->RenderGameEntities(frameInfo);

			m_renderer->EndSwapChainRenderPass(commandBuffer);
			m_renderer->EndFrame();
		}
	}

	vkDeviceWaitIdle(m_device->GetDevice());


	//m_buffer->Destroy(globalUBO);
	for (int i = 0; i < uboBuffers.size(); i++)
	{
		m_buffer->Destroy(uboBuffers[i]);
	}
}

void WindowApp::LoadCameraEntities()
{
	ecs::Entity camera = m_gameEntitySystem->CreateGameEntity(EntityType::Camera);

	m_cameraSystem->SetCurrentActiveCamera(camera);

	CameraTransformComponent& transformComponent = ecs::ComponentManager::GetComponent<CameraTransformComponent>(camera);
	transformComponent.Position = { 0.0f, 0.0f, 0.0f };
	
	CameraComponent& cameraComponent = ecs::ComponentManager::GetComponent<CameraComponent>(camera);

	m_cameraSystem->SetViewRotation(cameraComponent, transformComponent);
	m_cameraSystem->SetPerspectiveProjection(cameraComponent, glm::radians(50.0f), 1.0f, 0.1f, 10.0f);
}

void WindowApp::LoadGameEntities()
{
	std::unique_ptr<ModelData> cubeNoIndicesData = PrimitiveFactory::GenerateCubeNoIndices(
		{ 0.0f, 0.0f, 0.0f },
		{ glm::vec3(.9f, .9f, .9f), glm::vec3(.8f, .8f, .1f), glm::vec3(.9f, .6f, .1f), glm::vec3(.8f, .1f, .1f), glm::vec3(.1f, .1f, .8f), glm::vec3(.1f, .8f, .1f) }
	);

	//////////////////////////////////////////////////////////////////////////
	// Cube no Indices
	{
		ecs::Entity cubeNoIndices = m_gameEntitySystem->CreateGameEntity(EntityType::Object);

		m_modelSystem->LoadModel(cubeNoIndices, std::move(cubeNoIndicesData));

		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(cubeNoIndices);
		transformComponent.Position = { 0.0f, -1.0f, 3.0f };
		transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };
	}

	//////////////////////////////////////////////////////////////////////////
	// Flat Vase
	{
		std::unique_ptr<ModelData> flatVaseData = m_objLoader->LoadModel("Assets/Models/flat_vase.obj");
		ecs::Entity flatVase = m_gameEntitySystem->CreateGameEntity(EntityType::Object);

		m_modelSystem->LoadModel(flatVase, std::move(flatVaseData));

		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(flatVase);
		transformComponent.Position = { -1.5f, 0.5f, 3.0f };
		transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Cube
	{
		std::unique_ptr<ModelData> coloredCubeData = m_objLoader->LoadModel("Assets/Models/colored_cube.obj");
		ecs::Entity coloredCube = m_gameEntitySystem->CreateGameEntity(EntityType::Object);

		m_modelSystem->LoadModel(coloredCube, std::move(coloredCubeData));
		
		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(coloredCube);
		transformComponent.Position = { 0.0f, 0.0f, 3.0f };
		transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };
		
		// 	MaterialComponent& materialComponent = ecs::ComponentManager::GetComponent<MaterialComponent>(coloredCube);
		// 	materialComponent.Color = { 0.1f, 0.8f, 0.1f, 1.0f };
	}

	//////////////////////////////////////////////////////////////////////////
	// Smooth Vase
	{
		std::unique_ptr<ModelData> smoothVaseData = m_objLoader->LoadModel("Assets/Models/smooth_vase.obj");
		ecs::Entity smoothVase = m_gameEntitySystem->CreateGameEntity(EntityType::Object);

		m_modelSystem->LoadModel(smoothVase, std::move(smoothVaseData));

		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(smoothVase);
		transformComponent.Position = { 1.5f, 0.5f, 3.0f };
		transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };
	}
}

void WindowApp::UnloadGameEntities()
{
	m_modelSystem->UnloadModels();
	m_gameEntitySystem->DestroyGameEntities();
}