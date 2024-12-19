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

#define DESCRIPTOR_SET_COUNT 2

#define GLOBAL_BINDING_SCENE 0
#define GLOBAL_BINDING_LIGHT 1

#define GROUP_BINDING_OBJECT 0


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
};

// Light Uniform Buffer Object
struct LightUBO
{
	glm::vec4 LightPos{ 0.0f, -0.25f, 0.0f, 0.0f };
	glm::vec4 LightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
};

// Object Uniform Buffer Object
struct ObjectUBO
{
	glm::mat4 ModelMatrix{ 1.0f };
};


ViewerApp::ViewerApp(Config& _config) :
	VesperApp(_config)
{
	m_window = std::make_unique<ViewerWindow>(_config.WindowWidth, _config.WindowHeight, _config.WindowName);

	m_device = std::make_unique<Device>(*m_window);
	m_renderer = std::make_unique<Renderer>(*m_window, *m_device);

	const uint32 minUboAlignment = static_cast<uint32>(m_device->GetLimits().minUniformBufferOffsetAlignment);
	const uint32 minSboAlignment = static_cast<uint32>(m_device->GetLimits().minStorageBufferOffsetAlignment);

	const uint32 uboAlignedSize = m_buffer->GetAlignment<uint32>(sizeof(ObjectUBO), minUboAlignment);


	m_globalPool = DescriptorPool::Builder(*m_device)
		.SetMaxSets(SwapChain::kMaxFramesInFlight * DESCRIPTOR_SET_COUNT)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::kMaxFramesInFlight * DESCRIPTOR_SET_COUNT)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SwapChain::kMaxFramesInFlight * DESCRIPTOR_SET_COUNT)
		//.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SwapChain::kMaxFramesInFlight)
		.Build();

	//
	// I create 2 set layout for now:
	// 
	// 1. Global, where draws once per frame, which is the view projection + ambient color
	// 2. Group, which draws once per group, for now object matrices, using dynamic buffer
	// 
	// 
	// The first is Global (m_globalSetLayout) and has 2 bindings uniform buffers
	// 
	// 0. present in vertex & fragment , containing { mat4 projectionMatrix, mat4 viewMatrix; vec4 ambientLightColor; } - we can call this "Scene"
	// 1. present in fragment, containing { vec4 pointLightPosition; vec4 pointLightColor; } - we can call this "Light"
	// NOTE: the binding 1 should be later added to a "per object" descriptor layout, but for now here is fine.
	// 
	// 
	// The second is Group (m_groupSetLayout) and has 1 binding dynamic uniform buffer
	// 
	// 0. present in vertex, containg { mat4 modelMatrix; } - we can call this "Object"
	// 

	m_globalSetLayout = DescriptorSetLayout::Builder(*m_device)
		.AddBinding(GLOBAL_BINDING_SCENE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(GLOBAL_BINDING_LIGHT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)	// this has to be moved in a per object specific
		.Build();

	m_groupSetLayout = DescriptorSetLayout::Builder(*m_device)
		.AddBinding(GROUP_BINDING_OBJECT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT)
		.Build();

	m_gameEntitySystem = std::make_unique<GameEntitySystem>(*this);
	m_modelSystem = std::make_unique<ModelSystem>(*this, *m_device);

	m_simpleRenderSystem = std::make_unique<SimpleRenderSystem>(*this , *m_device, m_renderer->GetSwapChainRenderPass(),
		m_globalSetLayout->GetDescriptorSetLayout(),
		m_groupSetLayout->GetDescriptorSetLayout(),
		uboAlignedSize);

	m_cameraSystem = std::make_unique<CameraSystem>(*this);
	m_objLoader = std::make_unique<ObjLoader>(*m_device);
	m_buffer = std::make_unique<Buffer>(*m_device);

	m_keyboardController = std::make_unique<KeyboardMovementCameraController>(*this);

	m_mouseController = std::make_unique<MouseLookCameraController>();
	m_mouseController->SetMouseCallback(this, m_window->GetWindow());

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
	std::vector<BufferComponent> sceneUboBuffers(SwapChain::kMaxFramesInFlight);
	std::vector<BufferComponent> lightUboBuffers(SwapChain::kMaxFramesInFlight);
	std::vector<BufferComponent> objectUboBuffers(SwapChain::kMaxFramesInFlight);

	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		sceneUboBuffers[i] = m_buffer->Create<BufferComponent>(
			sizeof(SceneUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, //VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			1,
			true
		);

		lightUboBuffers[i] = m_buffer->Create<BufferComponent>(
			sizeof(LightUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, //VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			1,
			true
		);

		objectUboBuffers[i] = m_buffer->Create<BufferComponent>(
			sizeof(ObjectUBO),
			m_simpleRenderSystem->GetObjectCount(),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, //| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, //VMA_MEMORY_USAGE_AUTO_PREFER_HOST, //VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,//VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			m_simpleRenderSystem->GetAlignedSizeUBO(),
			true
		);
	}

	std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::kMaxFramesInFlight);
	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		auto sceneBufferInfo = m_buffer->GetDescriptorInfo(sceneUboBuffers[i]);
		auto lightBufferInfo = m_buffer->GetDescriptorInfo(lightUboBuffers[i]);

		DescriptorWriter(*m_globalSetLayout, *m_globalPool)
			.WriteBuffer(GLOBAL_BINDING_SCENE, &sceneBufferInfo)
			.WriteBuffer(GLOBAL_BINDING_LIGHT, &lightBufferInfo)
			.Build(globalDescriptorSets[i]);
	}

	std::vector<VkDescriptorSet> groupDescriptorSets(SwapChain::kMaxFramesInFlight);
	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		auto objectBufferInfo = m_buffer->GetDescriptorInfo(objectUboBuffers[i]);

		DescriptorWriter(*m_groupSetLayout, *m_globalPool)
			.WriteBuffer(GROUP_BINDING_OBJECT, &objectBufferInfo)
			.Build(groupDescriptorSets[i]);
	}

	auto currentTime = std::chrono::high_resolution_clock::now();

	SceneUBO sceneUBO;
	LightUBO lightUBO;
	ObjectUBO objectUBO;

	CameraComponent activeCameraComponent;
	CameraTransformComponent activeCameraTransformComponent;

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

			FrameInfo frameInfo { frameIndex, frameTime, commandBuffer, globalDescriptorSets[frameIndex], groupDescriptorSets[frameIndex] };
			
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

			m_simpleRenderSystem->Update(frameInfo);
			 
			m_cameraSystem->Update(aspectRatio);
			m_cameraSystem->GetActiveCameraData(0, activeCameraComponent, activeCameraTransformComponent);

			sceneUBO.ProjectionMatrix = activeCameraComponent.ProjectionMatrix;
			sceneUBO.ViewMatrix = activeCameraComponent.ViewMatrix;
			
			m_buffer->WriteToBuffer(sceneUboBuffers[frameIndex], &sceneUBO);
			m_buffer->Flush(sceneUboBuffers[frameIndex]);

			m_buffer->WriteToBuffer(lightUboBuffers[frameIndex], &lightUBO);
			m_buffer->Flush(lightUboBuffers[frameIndex]);

			// For instance, add here before the swap chain:
			// begin off screen shadow pass
			//	render shadow casting objects
			// end off screen shadow pass

			m_renderer->BeginSwapChainRenderPass(commandBuffer);

			for (auto gameEntity : ecs::IterateEntitiesWithAll<DynamicOffsetComponent, RenderComponent>(entityManager, componentManager))
			{
				const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(gameEntity);
				const RenderComponent& renderComponent = componentManager.GetComponent<RenderComponent>(gameEntity);

				objectUBO.ModelMatrix = renderComponent.ModelMatrix;
				 
				m_buffer->WriteToIndex(objectUboBuffers[frameIndex], &objectUBO, dynamicOffsetComponent.DynamicOffsetIndex);
				m_buffer->FlushIndex(objectUboBuffers[frameIndex], dynamicOffsetComponent.DynamicOffsetIndex);
			}

			m_simpleRenderSystem->Render(frameInfo);

			m_renderer->EndSwapChainRenderPass(commandBuffer);
			m_renderer->EndFrame();
		}
	}

	vkDeviceWaitIdle(m_device->GetDevice());

	for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
	{
		m_buffer->Destroy(sceneUboBuffers[i]);
		m_buffer->Destroy(lightUboBuffers[i]);
		m_buffer->Destroy(objectUboBuffers[i]);
	}
}

void ViewerApp::LoadCameraEntities()
{
	{
		ecs::Entity camera = m_gameEntitySystem->CreateGameEntity(EntityType::Camera);

		CameraTransformComponent& transformComponent = GetComponentManager().GetComponent<CameraTransformComponent>(camera);
		transformComponent.Position = { 0.0f, 0.0f, -3.0f };

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
		transformComponent.Position = { 0.0f, -1.0f, 0.0f };
		transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_simpleRenderSystem->RegisterEntity(cubeNoIndices);

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
		std::unique_ptr<ModelData> coloredCubeData = m_objLoader->LoadModel("Assets/Models/colored_cube.obj");
		ecs::Entity coloredCube = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

		m_modelSystem->LoadModel(coloredCube, std::move(coloredCubeData));

		TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(coloredCube);
		transformComponent.Position = { 0.0f, 0.0f, 0.0f };
		transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_simpleRenderSystem->RegisterEntity(coloredCube);

		// 	MaterialComponent& materialComponent = GetComponentManager().GetComponent<MaterialComponent>(coloredCube);
		// 	materialComponent.Color = { 0.1f, 0.8f, 0.1f, 1.0f };

		// test
		GetComponentManager().AddComponent<RotationComponent>(coloredCube);

		static const float radPerFrame = 0.00174533f;     // 0.1 deg
		RotationComponent& rotateComponent = GetComponentManager().GetComponent<RotationComponent>(coloredCube);
		rotateComponent.RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		rotateComponent.RadiantPerFrame = radPerFrame;
	}

	//////////////////////////////////////////////////////////////////////////
	// Flat Vase
	{
		std::unique_ptr<ModelData> flatVaseData = m_objLoader->LoadModel("Assets/Models/flat_vase.obj");
		ecs::Entity flatVase = m_gameEntitySystem->CreateGameEntity(EntityType::Renderable);

		m_modelSystem->LoadModel(flatVase, std::move(flatVaseData));

		TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(flatVase);
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

		TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(smoothVase);
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

		TransformComponent& transformComponent = GetComponentManager().GetComponent<TransformComponent>(quad);
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