// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\App\window_handle.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "vulkan/vulkan.h"

#include <string>
#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API WindowHandle
{
public:
	WindowHandle(int32 _width, int32 _height, std::string _name);
	virtual ~WindowHandle();

public:
	VESPERENGINE_INLINE bool IsValidationLayersEnabled() { return m_enableValidationLayers; }
	VESPERENGINE_INLINE VkExtent2D GetExtent() { return{ static_cast<uint32>(m_width) , static_cast<uint32>(m_height) }; }

	VESPERENGINE_INLINE bool WasWindowResized() { return m_frameBufferResized; }
	VESPERENGINE_INLINE void ResetWindowResizedFlag() { m_frameBufferResized = false; }
	
public:
	virtual void CreateWindowSurface(VkInstance _instance, VkSurfaceKHR* _surface) = 0;
	virtual std::vector<const char*> GetRequiredExtensions() = 0;
	virtual void WaitEvents() = 0;

protected:
	int32 m_width;
	int32 m_height;
	std::string m_name;

	bool m_frameBufferResized = false;

private:
#ifdef NDEBUG
	const bool m_enableValidationLayers = false;
#else
	const bool m_enableValidationLayers = true;
#endif
};

VESPERENGINE_NAMESPACE_END
