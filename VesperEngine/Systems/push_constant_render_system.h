#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/pipeline.h"
#include "Backend/frame_info.h"

#include "Components/camera_components.h"

#include "Systems/base_render_system.h"

#include "App/vesper_app.h"
#include "App/window_handle.h"

#include <memory>

#include "ECS/ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN


class VESPERENGINE_API PushConstantRenderSystem final : public BaseRenderSystem
{
public:
	PushConstantRenderSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass, VkDescriptorSetLayout _globalDescriptorSetLayout);
	~PushConstantRenderSystem();

	PushConstantRenderSystem(const PushConstantRenderSystem&) = delete;
	PushConstantRenderSystem& operator=(const PushConstantRenderSystem&) = delete;

public:
	void RegisterEntity(ecs::Entity _entity) const;
	void UnregisterEntity(ecs::Entity _entity) const;
	void UnregisterEntities() const;

protected:
	virtual void UpdateFrame(const FrameInfo& _frameInfo) override;
	virtual void RenderFrame(const FrameInfo& _frameInfo) override;
	virtual void SetupPipeline(PipelineConfigInfo& _pipelineConfig) override;

private:
	VesperApp& m_app;
};

VESPERENGINE_NAMESPACE_END
