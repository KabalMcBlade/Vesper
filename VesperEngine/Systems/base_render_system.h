#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"

#include "Components/graphics_components.h"
#include "Components/camera_components.h"

#include "Backend/pipeline.h"

#include "Backend/frame_info.h"

#include "Systems/core_render_system.h"


VESPERENGINE_NAMESPACE_BEGIN


/**
 * Vulkan Canonical Viewing Volume
 * x is in range of -1 to 1
 * y is in range of -1 to 1
 * z is in range of 0 to 1
 * This is right hand coordinate system
 * Not that positive x is pointing right, positive y is down and positive z is point in to the screen (from screen away)
 * 
 */

class VESPERENGINE_API BaseRenderSystem : public CoreRenderSystem
{
public:
	BaseRenderSystem(Device& _device);
	virtual ~BaseRenderSystem() = default;

	BaseRenderSystem(const BaseRenderSystem&) = delete;
	BaseRenderSystem& operator=(const BaseRenderSystem&) = delete;
	
public:
	void Update(const FrameInfo& _frameInfo);
	void Render(const FrameInfo& _frameInfo);

protected:
	virtual void UpdateFrame(const FrameInfo& _frameInfo) {}
	virtual void RenderFrame(const FrameInfo& _frameInfo) {}
};

VESPERENGINE_NAMESPACE_END
