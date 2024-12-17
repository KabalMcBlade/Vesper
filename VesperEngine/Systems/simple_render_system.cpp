#include "pch.h"
#include "simple_render_system.h"

#include <array>
#include <stdexcept>
#include <iostream>

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <gtx/quaternion.hpp>

#include "Components/graphics_components.h"
#include "Components/object_components.h"

#include "App/vesper_app.h"


VESPERENGINE_NAMESPACE_BEGIN

SimpleRenderSystem::SimpleRenderSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass,
	VkDescriptorSetLayout _globalDescriptorSetLayout,
	VkDescriptorSetLayout _groupDescriptorSetLayout,
	uint32 _alignedSizeUBO)
	: m_app(_app)
	, BaseRenderSystem{ _device }
{
	CreatePipelineLayout(std::vector<VkDescriptorSetLayout>{ _globalDescriptorSetLayout, _groupDescriptorSetLayout });
	CreatePipeline(_renderPass);

	m_alignedSizeUBO = _alignedSizeUBO;
}

SimpleRenderSystem::~SimpleRenderSystem()
{
}

void SimpleRenderSystem::RegisterEntity(ecs::Entity _entity) const
{
	// just to ensure it has, but the RenderComponent should be already present
	if (!m_app.GetComponentManager().HasComponents<RenderComponent>(_entity))
	{
		m_app.GetComponentManager().AddComponent<RenderComponent>(_entity);
	}

	// don't add a check here, so we avoid to add twice the same entity, it will trigger an assert
	m_app.GetComponentManager().AddComponent<DynamicOffsetComponent>(_entity);

	DynamicOffsetComponent& dynamicUniformBufferComponent = m_app.GetComponentManager().GetComponent<DynamicOffsetComponent>(_entity);
	dynamicUniformBufferComponent.DynamicOffsetIndex = m_internalCounter;
	dynamicUniformBufferComponent.DynamicOffset = m_internalCounter * m_alignedSizeUBO;

	++m_internalCounter;
}

void SimpleRenderSystem::UnregisterEntity(ecs::Entity _entity) const
{
	if (m_app.GetComponentManager().HasComponents<DynamicOffsetComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<DynamicOffsetComponent>(_entity);
	}
}

void SimpleRenderSystem::UnregisterEntities() const
{
	for (auto entity : ecs::IterateEntitiesWithAll<DynamicOffsetComponent>())
	{
		UnregisterEntity(entity);
	}
}

void SimpleRenderSystem::UpdateFrame(const FrameInfo& _frameInfo)
{
	for (auto gameEntity : ecs::IterateEntitiesWithAll<RenderComponent, TransformComponent>())
	{
		TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(gameEntity);
		RenderComponent& renderComponent = m_app.GetComponentManager().GetComponent<RenderComponent>(gameEntity);

		renderComponent.ModelMatrix = glm::translate(glm::mat4{ 1.0f }, transformComponent.Position);
		renderComponent.ModelMatrix = renderComponent.ModelMatrix * glm::toMat4(transformComponent.Rotation);
		renderComponent.ModelMatrix = glm::scale(renderComponent.ModelMatrix, transformComponent.Scale);
	}
}

void SimpleRenderSystem::RenderFrame(const FrameInfo& _frameInfo)
{
	// 1. Render whatever has vertex buffers and index buffer
	for (auto gameEntity : ecs::IterateEntitiesWithAll<DynamicOffsetComponent, VertexBufferComponent, IndexBufferComponent>())
	{
		const DynamicOffsetComponent& dynamicOffsetComponent = m_app.GetComponentManager().GetComponent<DynamicOffsetComponent>(gameEntity);

		const VertexBufferComponent& vertexBufferComponent = m_app.GetComponentManager().GetComponent<VertexBufferComponent>(gameEntity);
		const IndexBufferComponent& indexBufferComponent = m_app.GetComponentManager().GetComponent<IndexBufferComponent>(gameEntity);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
			m_pipelineLayout,
			1,
			1,
			&_frameInfo.GroupDescriptorSet,
			1,
			&dynamicOffsetComponent.DynamicOffset
		);

		Bind(vertexBufferComponent, indexBufferComponent, _frameInfo.CommandBuffer);
		Draw(indexBufferComponent, _frameInfo.CommandBuffer);
	}

	// 2. Render only entities having Vertex buffers only
	for (auto gameEntity : ecs::IterateEntitiesWithAll<DynamicOffsetComponent, VertexBufferComponent, NotIndexBufferComponent>())
	{
		const DynamicOffsetComponent& dynamicOffsetComponent = m_app.GetComponentManager().GetComponent<DynamicOffsetComponent>(gameEntity);

		const VertexBufferComponent& vertexBufferComponent = m_app.GetComponentManager().GetComponent<VertexBufferComponent>(gameEntity);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
			m_pipelineLayout,
			1,
			1,
			&_frameInfo.GroupDescriptorSet,
			1,
			&dynamicOffsetComponent.DynamicOffset
		);

		Bind(vertexBufferComponent, _frameInfo.CommandBuffer);
		Draw(vertexBufferComponent, _frameInfo.CommandBuffer);
	}
}

void SimpleRenderSystem::SetupePipeline(PipelineConfigInfo& _pipelineConfig)
{
	m_pipeline = std::make_unique<Pipeline>(
		m_device,
		std::vector{ ShaderInfo{"Assets/Shaders/simple_shader.vert.spv", ShaderType::Vertex}, ShaderInfo{"Assets/Shaders/simple_shader.frag.spv", ShaderType::Fragment}, },
		_pipelineConfig
	);
}

VESPERENGINE_NAMESPACE_END
