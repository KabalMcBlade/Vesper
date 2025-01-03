#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/pipeline.h"
#include "Backend/frame_info.h"
#include "Backend/descriptors.h"

#include "Components/camera_components.h"

#include "Systems/base_render_system.h"

#include "App/window_handle.h"

#include <memory>

#include "ECS/ECS/ecs.h"


/// <summary>
/// The SimpleRenderSystem is indeed a Phong renderer
/// This will retain all the data in need to process correctly the shader.
/// Will be renamed in next iteration!
/// </summary>

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;

class VESPERENGINE_API SimpleRenderSystem final : public BaseRenderSystem
{
public:
	SimpleRenderSystem(VesperApp& _app, Device& _device, DescriptorPool& _globalDescriptorPool, VkRenderPass _renderPass,
		VkDescriptorSetLayout _globalDescriptorSetLayout,
		VkDescriptorSetLayout _groupDescriptorSetLayout,
		uint32 _alignedSizeUBO);
	~SimpleRenderSystem();

	SimpleRenderSystem(const SimpleRenderSystem&) = delete;
	SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

public:
	VESPERENGINE_INLINE uint32 GetObjectCount() const { return m_internalCounter; }
	VESPERENGINE_INLINE uint32 GetAlignedSizeUBO() const { return m_alignedSizeUBO; }

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
	DescriptorPool& m_globalDescriptorPool;
	std::unique_ptr<DescriptorSetLayout> m_materialSetLayout;
	uint32 m_alignedSizeUBO {0};
	mutable uint32 m_internalCounter {0};
};

VESPERENGINE_NAMESPACE_END
