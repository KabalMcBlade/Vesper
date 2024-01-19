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
	SimpleRenderSystem(Device& _device, VkRenderPass _renderPass, VkDescriptorSetLayout _globalDescriptorSetLayout, uint32 _sizePerObjectUBO);
	~SimpleRenderSystem();

	SimpleRenderSystem(const SimpleRenderSystem&) = delete;
	SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

public:
	VESPERENGINE_INLINE uint32 GetObjectCount() const { return m_internalCounter; }
	VESPERENGINE_INLINE uint32 GetDynamicAlignmentObjectUBO() const { return m_dynamicAlignmentObjectUBO; }

public:
	void RegisterEntity(ecs::Entity _entity) const;
	void UnregisterEntity(ecs::Entity _entity) const;
	void UnregisterEntities() const;

protected:
	virtual void UpdateFrame(const FrameInfo& _frameInfo) override;
	virtual void RenderFrame(const FrameInfo& _frameInfo) override;
	virtual void SetupePipeline(PipelineConfigInfo& _pipelineConfig) override;

private:
	uint32 m_dynamicAlignmentObjectUBO {0};
	mutable uint32 m_internalCounter {0};
};

VESPERENGINE_NAMESPACE_END
