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

class VESPERENGINE_DLL PushConstantRenderSystem final : public BaseRenderSystem
{
public:
	PushConstantRenderSystem(Device& _device, VkRenderPass _renderPass, VkDescriptorSetLayout _globalDescriptorSetLayout);
	~PushConstantRenderSystem();

	PushConstantRenderSystem(const PushConstantRenderSystem&) = delete;
	PushConstantRenderSystem& operator=(const PushConstantRenderSystem&) = delete;

public:
	void RegisterEntity(ecs::Entity _entity) const;
	void UnregisterEntity(ecs::Entity _entity) const;
	void UnregisterEntities() const;

protected:
	virtual void UpdateFrame(FrameInfo& _frameInfo) override;
	virtual void RenderFrame(FrameInfo& _frameInfo) override;
	virtual void SetupePipeline(PipelineConfigInfo& _pipelineConfig) override;
};

VESPERENGINE_NAMESPACE_END
