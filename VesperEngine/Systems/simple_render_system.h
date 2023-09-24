#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/pipeline.h"
#include "Systems/core_render_system.h"
#include "App/window_handle.h"

#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL SimpleRenderSystem final : public CoreRenderSystem
{
public:
	SimpleRenderSystem(Device& _device, VkRenderPass _renderPass);
	~SimpleRenderSystem();

	SimpleRenderSystem(const SimpleRenderSystem&) = delete;
	SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

public:
	void RenderGameEntities(VkCommandBuffer _commandBuffer);

private:
	void CreatePipelineLayout();
	void CreatePipeline(VkRenderPass _renderPass);

private:
	std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
};

VESPERENGINE_NAMESPACE_END
