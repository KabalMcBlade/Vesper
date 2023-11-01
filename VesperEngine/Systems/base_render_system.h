#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/device.h"
#include "Components/graphics_components.h"
#include "Components/camera_components.h"
#include "Backend/pipeline.h"
#include "Backend/frame_info.h"


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

class VESPERENGINE_DLL BaseRenderSystem
{
public:
	BaseRenderSystem(Device& _device);
	virtual ~BaseRenderSystem() = default;

	BaseRenderSystem(const BaseRenderSystem&) = delete;
	BaseRenderSystem& operator=(const BaseRenderSystem&) = delete;
	
public:
	void Render(FrameInfo& _frameInfo);
	void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts);
	void CreatePipeline(VkRenderPass _renderPass);

protected:
	virtual void RenderFrame(FrameInfo& _frameInfo) {}
	virtual void SetupePipeline(PipelineConfigInfo& _pipelineConfig) {};

protected:
	void Bind(VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const;
	void Bind(VertexBufferComponent& _vertexBufferComponent, IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const;

	void Draw(VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const;
	void Draw(IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const;

protected:
	Device& m_device;

protected:
	VkPipelineLayout m_pipelineLayout;
	std::unique_ptr<Pipeline> m_pipeline;
	std::vector<VkPushConstantRange> m_pushConstants;
};

VESPERENGINE_NAMESPACE_END
