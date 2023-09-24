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
	WindowApp(Config& _config);
	~WindowApp();

	WindowApp(const WindowApp&) = delete;
	WindowApp& operator=(const WindowApp&) = delete;

public:
	void Run();

private:
	void LoadGameEntities();
	void UnloadGameEntities();

private:
	std::unique_ptr<ViewerWindow> m_window;
	std::unique_ptr<Device> m_device;
	std::unique_ptr<Renderer> m_renderer;


	std::unique_ptr<GameEntityLoaderSystem> m_gameEntityLoaderSystem;
	std::unique_ptr<SimpleRenderSystem> m_simpleRenderSystem;

	// test for fun
	RainbowSystem m_rainbowSystem {60.0f};
};

