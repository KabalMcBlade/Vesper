#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/device.h"
#include "Components/graphics_components.h"
#include "Components/camera_components.h"


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

class VESPERENGINE_DLL CoreRenderSystem
{
public:
	CoreRenderSystem(Device& _device);
	virtual ~CoreRenderSystem() = default;

	CoreRenderSystem(const CoreRenderSystem&) = delete;
	CoreRenderSystem& operator=(const CoreRenderSystem&) = delete;

protected:
	virtual void Bind(VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const;
	virtual void Bind(VertexBufferComponent& _vertexBufferComponent, IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const;

	virtual void Draw(VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const;
	virtual void Draw(IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const;

protected:
	Device& m_device;
};

VESPERENGINE_NAMESPACE_END
