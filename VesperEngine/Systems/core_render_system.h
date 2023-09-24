#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/device.h"
#include "Components/core_components.h"

#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL CoreRenderSystem
{
public:
	CoreRenderSystem(Device& _device);
	virtual ~CoreRenderSystem();

	CoreRenderSystem(const CoreRenderSystem&) = delete;
	CoreRenderSystem& operator=(const CoreRenderSystem&) = delete;

protected:
	virtual void Bind(RenderComponent& _renderComponent, VkCommandBuffer _commandBuffer);
	virtual void Draw(RenderComponent& _renderComponent, VkCommandBuffer _commandBuffer);

protected:
	Device& m_device;
};

VESPERENGINE_NAMESPACE_END
