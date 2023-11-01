#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/pipeline.h"
#include "Backend/frame_info.h"
#include "Components/camera_components.h"
#include "Systems/base_render_system.h"
#include "App/window_handle.h"

#include <memory>

#include "ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL SimpleRenderSystem final : public BaseRenderSystem
{
public:
	SimpleRenderSystem(Device& _device, VkRenderPass _renderPass, VkDescriptorSetLayout _globalDescriptorSetLayout);
	~SimpleRenderSystem();

	SimpleRenderSystem(const SimpleRenderSystem&) = delete;
	SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

protected:
	virtual void RenderFrame(FrameInfo& _frameInfo) override;
	virtual void SetupePipeline(PipelineConfigInfo& _pipelineConfig) override;

	void TransformEntity(VkCommandBuffer _commandBuffer, ecs::Entity _entity, glm::mat4 _projectionView);
};

VESPERENGINE_NAMESPACE_END
