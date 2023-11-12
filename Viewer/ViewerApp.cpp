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
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// ONLY IF USING VESPER ENGINE AS DLL, IF USING AS STATIC LIB, DON'T ADD!
//#define VMA_IMPLEMENTATION
//#include <vma/vk_mem_alloc.h>


VESPERENGINE_USING_NAMESPACE

// special struct to just inform the system this entity can be rotated
struct RotationComponent
{
	glm::vec3 RotationAxis;
	float RadiantPerFrame;
};


// Scene Uniform Buffer Object
struct SceneUBO
{
	glm::mat4 ProjectionMatrix{ 1.0f };
	glm::mat4 ViewMatrix{ 1.0f };
	glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 0.5f };	// w is intensity
	glm::vec4 LightPos { 0.0f, -0.25f, 0.0f, 0.0f };
	glm::vec4 LightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
};


ViewerApp::ViewerApp(Config& _config) :
	VesperApp(_config)
{
	m_window = std::make_unique<ViewerWindow>(_config.WindowWidth, _config.WindowHeight, _config.WindowName);

	m_device = std::make_unique<Device>(*m_window);
	m_renderer = std::make_unique<Renderer>(*m_window, *m_device);

	// This should be managed as global pool per render system
	m_globalPool = DescriptorPool::Builder(*m_device)
		.SetMaxSets(SwapChain::kMaxFramesInFlight)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::kMaxFramesInFlight)
		//.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SwapChain::kMaxFramesInFlight)
		.Build();

	// This should be part of the highest render system, which is shared among all the possible shaders and so all the possible Render systems
	m_globalSetLayout = DescriptorSetLayout::Builder(*m_device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
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

	// test
	ecs::ComponentManager::RegisterComponent<RotationComponent>();

	LoadCameraEntities();
	LoadGameEntities();
}

ViewerApp::~ViewerApp()
{
	// test
	ecs::ComponentManager::UnregisterComponent<RotationComponent>();

	UnloadGameEntities();
}

void ViewerApp::Run()
{
	// This should be in separated class which should manage the "actual" game in terms of what render
// 	BufferComponent sceneUBO;
// 	m_buffer->Create(
// 		sceneUBO,
// 		sizeof(SceneUBO),
// 		SwapChain::kMaxFramesInFlight,
// 		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
// 		VMA_MEMORY_USAGE_AUTO, //VMA_MEMORY_USAGE_AUTO,//VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
// 		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
// 		m_device->GetProperties().limits.minUniformBufferOffsetAlignment,
// 		true
// 	);

	std::vector<BufferComponent> sceneUboBuffers(SwapChain::kMaxFramesInFlight);
	for (int i = 0; i < sceneUboBuffers.size(); i++) 
	{
		sceneUboBuffers[i] = m_buffer->Create<BufferComponent>(
			sizeof(SceneUBO),
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
		auto bufferInfo = m_buffer->GetDescriptorInfo(sceneUboBuffers[i]);
		DescriptorWriter(*m_globalSetLayout, *m_globalPool)
			.WriteBuffer(0, &bufferInfo)
			.Build(globalDescriptorSets[i]);
	}

	auto currentTime = std::chrono::high_resolution_clock::now();

	SceneUBO sceneUBO;
	CameraComponent activeCameraComponent;
	CameraTransformComponent activeCameraTransformComponent;

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
			
			const float aspectRatio = m_renderer->GetAspectRatio();

			m_simpleRenderSystem->Update(frameInfo);

			//////////////////////////////////////////////////////////////////////////
			// test

			for (auto gameEntity : ecs::IterateEntitiesWithAll<TransformComponent, RotationComponent>())
			{
				RotationComponent& rotateComponent = ecs::ComponentManager::GetComponent<RotationComponent>(gameEntity);
				TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(gameEntity);

				// Add random rotation, for testing UBO
				const glm::quat& prevRot = transformComponent.Rotation;
				glm::quat currRot = glm::angleAxis(rotateComponent.RadiantPerFrame, rotateComponent.RotationAxis);
				transformComponent.Rotation = prevRot * currRot;
			}


			//////////////////////////////////////////////////////////////////////////


			m_cameraSystem->Update(aspectRatio);
			m_cameraSystem->GetActiveCameraData(0, activeCameraComponent, activeCameraTransformComponent);

			sceneUBO.ProjectionMatrix = activeCameraComponent.ProjectionMatrix;
			sceneUBO.ViewMatrix = activeCameraComponent.ViewMatrix;


			//m_buffer->WriteToIndex(uboBuffers[frameIndex], &ubo, frameIndex);
			//m_buffer->FlushIndex(uboBuffers[frameIndex], frameIndex);
			m_buffer->WriteToBuffer(sceneUboBuffers[frameIndex], &sceneUBO);
			m_buffer->Flush(sceneUboBuffers[frameIndex]);


			// For instance, add here before the swap chain:
			// begin off screen shadow pass
			//	render shadow casting objects
			// end off screen shadow pass

			m_renderer->BeginSwapChainRenderPass(commandBuffer);

			m_simpleRenderSystem->Render(frameInfo);

			m_renderer->EndSwapChainRenderPass(commandBuffer);
			m_renderer->EndFrame();
		}
	}

	vkDeviceWaitIdle(m_device->GetDevice());


	//m_buffer->Destroy(globalUBO);
	for (int i = 0; i < sceneUboBuffers.size(); i++)
	{
		m_buffer->Destroy(sceneUboBuffers[i]);
	}
}

void ViewerApp::LoadCameraEntities()
{
	{
		ecs::Entity camera = m_gameEntitySystem->CreateGameEntity(EntityType::Camera);

		CameraTransformComponent& transformComponent = ecs::ComponentManager::GetComponent<CameraTransformComponent>(camera);
		transformComponent.Position = { 0.0f, 0.0f, -3.0f };

		CameraComponent& cameraComponent = ecs::ComponentManager::GetComponent<CameraComponent>(camera);

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

		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(cubeNoIndices);
		transformComponent.Position = { 0.0f, -1.0f, 0.0f };
		transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_simpleRenderSystem->RegisterEntity(cubeNoIndices);

		// test
		ecs::ComponentManager::AddComponent<RotationComponent>(cubeNoIndices);

		static const float radPerFrame = 0.00174533f;     // 0.1 deg
		RotationComponent& rotateComponent = ecs::ComponentManager::GetComponent<RotationComponent>(cubeNoIndices);
		rotateComponent.RotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		rotateComponent.RadiantPerFrame = radPerFrame;
	}


	//////////////////////////////////////////////////////////////////////////
	// Cube
	{
		std::unique_ptr<ModelData> coloredCubeData = m_objLoader->LoadModel("Assets/Models/colored_cube.obj");
		ecs::Entity coloredCube = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

		m_modelSystem->LoadModel(coloredCube, std::move(coloredCubeData));

		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(coloredCube);
		transformComponent.Position = { 0.0f, 0.0f, 0.0f };
		transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_simpleRenderSystem->RegisterEntity(coloredCube);

		// 	MaterialComponent& materialComponent = ecs::ComponentManager::GetComponent<MaterialComponent>(coloredCube);
		// 	materialComponent.Color = { 0.1f, 0.8f, 0.1f, 1.0f };

		// test
		ecs::ComponentManager::AddComponent<RotationComponent>(coloredCube);

		static const float radPerFrame = 0.00174533f;     // 0.1 deg
		RotationComponent& rotateComponent = ecs::ComponentManager::GetComponent<RotationComponent>(coloredCube);
		rotateComponent.RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		rotateComponent.RadiantPerFrame = radPerFrame;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Flat Vase
	{
		std::unique_ptr<ModelData> flatVaseData = m_objLoader->LoadModel("Assets/Models/flat_vase.obj");
		ecs::Entity flatVase = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

		m_modelSystem->LoadModel(flatVase, std::move(flatVaseData));

		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(flatVase);
		transformComponent.Position = { -1.0f, 0.5f, 0.0f };
		transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_simpleRenderSystem->RegisterEntity(flatVase);
	}

	//////////////////////////////////////////////////////////////////////////
	// Smooth Vase
	{
		std::unique_ptr<ModelData> smoothVaseData = m_objLoader->LoadModel("Assets/Models/smooth_vase.obj");
		ecs::Entity smoothVase = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

		m_modelSystem->LoadModel(smoothVase, std::move(smoothVaseData));

		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(smoothVase);
		transformComponent.Position = { 1.0f, 0.5f, 0.0f };
		transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_simpleRenderSystem->RegisterEntity(smoothVase);
	}

	//////////////////////////////////////////////////////////////////////////
	// Plane / Quad
	{
		std::unique_ptr<ModelData> quadData = m_objLoader->LoadModel("Assets/Models/quad.obj");
		ecs::Entity quad = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

		m_modelSystem->LoadModel(quad, std::move(quadData));

		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(quad);
		transformComponent.Position = { 0.0f, 1.0f, 0.0f };
		transformComponent.Scale = { 3.0f, 1.0f, 3.0f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_simpleRenderSystem->RegisterEntity(quad);
	}
}

void ViewerApp::UnloadGameEntities()
{
	m_modelSystem->UnloadModels();
	m_simpleRenderSystem->UnregisterEntities();
	m_gameEntitySystem->DestroyGameEntities();
}