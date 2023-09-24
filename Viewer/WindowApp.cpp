#include "WindowApp.h"

#include <array>
#include <stdexcept>
#include <iostream>

// TEMP HERE
#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

VESPERENGINE_USING_NAMESPACE

WindowApp::WindowApp(Config& _config)
{
	m_window = std::make_unique<ViewerWindow>(
		_config.WindowWidth, _config.WindowHeight, _config.WindowName,
		_config.MaxEntities, _config.MaxComponentsPerEntity);

	m_device = std::make_unique<Device>(*m_window);
	m_renderer = std::make_unique<Renderer>(*m_window, *m_device);

	m_gameEntityLoaderSystem = std::make_unique<GameEntityLoaderSystem>(*m_device);
	m_simpleRenderSystem = std::make_unique<SimpleRenderSystem>(*m_device, m_renderer->GetSwapChainRenderPass());

	LoadGameEntities();
}

WindowApp::~WindowApp()
{
	UnloadGameEntities();
}

void WindowApp::Run()
{
	while (!m_window->ShouldClose())
	{
		glfwPollEvents();

		auto commandBuffer = m_renderer->BeginFrame();
		if (commandBuffer != VK_NULL_HANDLE)
		{
			// For instance, add here before the swap chain:
			// begin off screen shadow pass
			//	render shadow casting objects
			// end off screen shadow pass

			m_renderer->BeginSwapChainRenderPass(commandBuffer);

			// for fun
			m_rainbowSystem.Update(1.0f/6.0f);

			m_simpleRenderSystem->RenderGameEntities(commandBuffer);

			m_renderer->EndSwapChainRenderPass(commandBuffer);

			m_renderer->EndFrame();
		}
	}

	vkDeviceWaitIdle(m_device->GetDevice());
}

void WindowApp::LoadGameEntities()
{
	std::vector<Vertex> vertices
	{
		{{0.0f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
	};


	ecs::Entity triangle = m_gameEntityLoaderSystem->CreateGameEntity();

	m_gameEntityLoaderSystem->LoadGameEntity(triangle, vertices);

	// I know these are present for sure
	TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(triangle);
	MaterialComponent& materialComponent = ecs::ComponentManager::GetComponent<MaterialComponent>(triangle);

	materialComponent.Color = { 0.1f, 0.8f, 0.1f, 1.0f };

	transformComponent.Position = { 0.0f, 0.0f, 0.0f, 1.0f };
	transformComponent.Scale = { 1.f, 1.0f, 1.0f, 0.0f };
	transformComponent.Rotation = glm::quat { 1.0f, 0.0f, 0.0f, 0.0f };
}


void WindowApp::UnloadGameEntities()
{
	m_gameEntityLoaderSystem->UnloadGameEntities();
	m_gameEntityLoaderSystem->DestroyGameEntities();
}