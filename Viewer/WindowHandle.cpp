// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\WindowHandle.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "WindowHandle.h"

#include <stdexcept>

VESPERENGINE_USING_NAMESPACE

ViewerWindow::ViewerWindow(uint32 _width, uint32 _height, std::string _name)
	: WindowHandle(_width, _height, _name)
{
	InitWindow();
}

ViewerWindow::~ViewerWindow()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void ViewerWindow::FramebufferResizedCallback(GLFWwindow* _window, int32 _width, int32 _height)
{
	auto windowHandle = reinterpret_cast<ViewerWindow*>(glfwGetWindowUserPointer(_window));

	windowHandle->m_frameBufferResized = true;
	windowHandle->m_width = _width;
	windowHandle->m_height = _height;
}

void ViewerWindow::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, FramebufferResizedCallback);
}

void ViewerWindow::CreateWindowSurface(VkInstance _instance, VkSurfaceKHR* _surface)
{
	if (glfwCreateWindowSurface(_instance, m_window, nullptr, _surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

std::vector<const char*> ViewerWindow::GetRequiredExtensions()
{
	uint32 glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (IsValidationLayersEnabled())
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void ViewerWindow::WaitEvents()
{
	glfwWaitEvents();
}