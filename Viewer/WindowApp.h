#pragma once

#include "WindowHandle.h"

// test for fun:
#include "RainbowSystem.h"

#include <memory>
#include <vector>

VESPERENGINE_USING_NAMESPACE

class WindowApp final
{
public:
	static constexpr int32 kWidth = 800;
	static constexpr int32 kHeight = 600;

public:
	WindowApp();
	~WindowApp();

	WindowApp(const WindowApp&) = delete;
	WindowApp& operator=(const WindowApp&) = delete;

public:
	void Run();

private:
	void LoadGameObjects();

private:
	ViewerWindow m_window{ kWidth, kHeight, "Hello Vulkan" };
	Device m_device{ m_window };
	Renderer m_renderer{ m_window, m_device };

	std::vector<GameObject> m_gameObjects;

	// test for fun
	RainbowSystem m_rainbowSystem {60.0f};
};

