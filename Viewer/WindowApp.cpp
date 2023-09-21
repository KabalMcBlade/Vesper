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

WindowApp::WindowApp()
{
	LoadGameObjects();
}

WindowApp::~WindowApp()
{

}

void WindowApp::Run()
{
	SimpleRenderSystem simpleRenderSystem { m_device, m_renderer.GetSwapChainRenderPass() };

	while (!m_window.ShouldClose())
	{
		glfwPollEvents();

		auto commandBuffer = m_renderer.BeginFrame();
		if (commandBuffer != VK_NULL_HANDLE)
		{
			// For instance, add here before the swap chain:
			// begin off screen shadow pass
			//	render shadow casting objects
			// end off screen shadow pass

			m_renderer.BeginSwapChainRenderPass(commandBuffer);

			m_rainbowSystem.Update(1.0f/6.0f, m_gameObjects);

			simpleRenderSystem.RenderGameObjects(commandBuffer, m_gameObjects);

			m_renderer.EndSwapChainRenderPass(commandBuffer);

			m_renderer.EndFrame();
		}
	}

	vkDeviceWaitIdle(m_device.GetDevice());
}

void WindowApp::LoadGameObjects()
{
	std::vector<Model::Vertex> vertices
	{
		{{0.0f, -0.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
	};

	//m_model = std::make_unique<Model>(m_device, vertices);

	auto model = std::make_shared<Model>(m_device, vertices);

	auto triangle = GameObject::CreateGameObject();
	triangle.m_model = model;
	triangle.m_color = { 0.1f, 0.8f, 0.1f, 1.0f };
	triangle.m_transform2D.translation.x = 0.2f;
	triangle.m_transform2D.scale = { 2.f, .5f, 1.0f, 1.0f };
	triangle.m_transform2D.rotation = 0.25f * glm::two_pi<float>();

	m_gameObjects.push_back(std::move(triangle));
}