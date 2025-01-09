// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\WindowHandle.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#define GLFW_INCLUDE_VULKAN
#include "ThirdParty/glfw/include/glfw3.h"

#include "vesper.h"

#include <string>


VESPERENGINE_USING_NAMESPACE

class ViewerWindow final : public WindowHandle
{
public:
	ViewerWindow(uint32 _width, uint32 _height, std::string _name);
	virtual ~ViewerWindow();

	// To avoid copy, because the GLFWwindow is a pointer and we rely on this very class to be on the stack
	ViewerWindow(const ViewerWindow&) = delete;
	ViewerWindow&operator=(const ViewerWindow&) = delete;

public:
	virtual void CreateWindowSurface(VkInstance _instance, VkSurfaceKHR* _surface) override;
	virtual std::vector<const char*> GetRequiredExtensions() override;
	virtual void WaitEvents() override;

public:
	VESPERENGINE_INLINE GLFWwindow* GetWindow() { return m_window; }
	VESPERENGINE_INLINE bool ShouldClose() { return glfwWindowShouldClose(m_window); }

private:
	static void FramebufferResizedCallback(GLFWwindow* _window, int32 _width, int32 _height);

private:
	void InitWindow();

	GLFWwindow *m_window;
};

