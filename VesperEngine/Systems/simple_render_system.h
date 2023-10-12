#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/pipeline.h"
#include "Backend/frame_info.h"
#include "Components/camera_components.h"
#include "Systems/core_render_system.h"
#include "App/window_handle.h"

#include <memory>

#include "ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL SimpleRenderSystem final : public CoreRenderSystem
{
public:
	SimpleRenderSystem(Device& _device, VkRenderPass _renderPass, VkDescriptorSetLayout _globalDescriptorSetLayout);
	~SimpleRenderSystem();

	SimpleRenderSystem(const SimpleRenderSystem&) = delete;
	SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

public:
	void RenderGameEntities(FrameInfo& _frameInfo);

private:
	void CreatePipelineLayout(VkDescriptorSetLayout _globalDescriptorSetLayout);
	void CreatePipeline(VkRenderPass _renderPass);

	void TransformEntity(VkCommandBuffer _commandBuffer, ecs::Entity _entity, glm::mat4 _projectionView);

private:
	std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
};

VESPERENGINE_NAMESPACE_END
