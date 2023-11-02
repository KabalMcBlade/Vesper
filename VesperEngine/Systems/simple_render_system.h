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
