#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/device.h"
#include "Components/graphics_components.h"
#include "Components/camera_components.h"
#include "Backend/pipeline.h"
#include "Backend/frame_info.h"


VESPERENGINE_NAMESPACE_BEGIN


class VESPERENGINE_API CoreRenderSystem
{
public:
	CoreRenderSystem(Device& _device);
	virtual ~CoreRenderSystem();

	CoreRenderSystem(const CoreRenderSystem&) = delete;
	CoreRenderSystem& operator=(const CoreRenderSystem&) = delete;

public:
	void CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& _descriptorSetLayouts);
	void CreatePipeline(VkRenderPass _renderPass);

protected:
	virtual void SetupPipeline(PipelineConfigInfo& _pipelineConfig) {};

protected:
	void PushConstants(VkCommandBuffer _commandBuffer, const uint32 _pushConstantIndex, const void* _pushConstantValue) const;
	void PushConstants(VkCommandBuffer _commandBuffer, std::vector<const void*> _pushConstantValues) const;

	void Bind(const VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer) const;
	void Bind(const VertexBufferComponent& _vertexBufferComponent, const IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer) const;

	void Draw(const VertexBufferComponent& _vertexBufferComponent, VkCommandBuffer _commandBuffer, uint32 _instanceCount = 1) const;
	void Draw(const IndexBufferComponent& _indexBufferComponent, VkCommandBuffer _commandBuffer, uint32 _instanceCount = 1) const;

protected:
	Device& m_device;
	VkPipelineLayout m_pipelineLayout;
	std::unique_ptr<Pipeline> m_pipeline;
	std::vector<VkPushConstantRange> m_pushConstants;
};

VESPERENGINE_NAMESPACE_END
