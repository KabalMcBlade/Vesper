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

#include "ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

SimpleRenderSystem::SimpleRenderSystem(Device& _device, VkRenderPass _renderPass, VkDescriptorSetLayout _globalDescriptorSetLayout, uint32 _sizePerObjectUBO)
	: BaseRenderSystem{ _device }
{
	CreatePipelineLayout(std::vector<VkDescriptorSetLayout>{ _globalDescriptorSetLayout });
	CreatePipeline(_renderPass);

	// need to align the size for the object buffer, since we are using 
	// Calculate required alignment based on minimum device offset alignment VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
	uint32 minUboAlignment = static_cast<uint32>(m_device.GetLimits().minUniformBufferOffsetAlignment);
	m_dynamicAlignmentObjectUBO = _sizePerObjectUBO;
	if (minUboAlignment > 0)
	{
		m_dynamicAlignmentObjectUBO = (m_dynamicAlignmentObjectUBO + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
}

SimpleRenderSystem::~SimpleRenderSystem()
{
}

void SimpleRenderSystem::RegisterEntity(ecs::Entity _entity) const
{
	// just to ensure it has, but the RenderComponent should be already present
	if (!ecs::ComponentManager::HasComponents<RenderComponent>(_entity))
	{
		ecs::ComponentManager::AddComponent<RenderComponent>(_entity);
	}

	RenderComponent& renderComponent = ecs::ComponentManager::GetComponent<RenderComponent>(_entity);
	renderComponent.DynamicOffsetIndex = m_internalCounter;
	renderComponent.DynamicOffset = m_internalCounter * m_dynamicAlignmentObjectUBO;
	++m_internalCounter;
}

void SimpleRenderSystem::UnregisterEntity(ecs::Entity _entity) const
{
	// Do nothing here
}

void SimpleRenderSystem::UnregisterEntities() const
{
	for (auto entity : ecs::IterateEntitiesWithAll<RenderComponent>())
	{
		UnregisterEntity(entity);
	}
}

void SimpleRenderSystem::UpdateFrame(FrameInfo& _frameInfo)
{
	for (auto gameEntity : ecs::IterateEntitiesWithAll<RenderComponent, TransformComponent>())
	{
		TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(gameEntity);
		RenderComponent& renderComponent = ecs::ComponentManager::GetComponent<RenderComponent>(gameEntity);

		renderComponent.ModelMatrix = glm::translate(glm::mat4{ 1.0f }, transformComponent.Position);
		renderComponent.ModelMatrix = renderComponent.ModelMatrix * glm::toMat4(transformComponent.Rotation);
		renderComponent.ModelMatrix = glm::scale(renderComponent.ModelMatrix, transformComponent.Scale);
	}
}

void SimpleRenderSystem::RenderFrame(FrameInfo& _frameInfo)
{
	// 1. Render whatever has vertex buffers and index buffer
	for (auto gameEntity : ecs::IterateEntitiesWithAll<RenderComponent, VertexBufferComponent, IndexBufferComponent>())
	{
		const RenderComponent& renderComponent = ecs::ComponentManager::GetComponent<RenderComponent>(gameEntity);

		const VertexBufferComponent& vertexBufferComponent = ecs::ComponentManager::GetComponent<VertexBufferComponent>(gameEntity);
		const IndexBufferComponent& indexBufferComponent = ecs::ComponentManager::GetComponent<IndexBufferComponent>(gameEntity);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
			m_pipelineLayout,
			0,
			1,
			&_frameInfo.GlobalDescriptorSet,
			1,
			&renderComponent.DynamicOffset
		);


		Bind(vertexBufferComponent, indexBufferComponent, _frameInfo.CommandBuffer);
		Draw(indexBufferComponent, _frameInfo.CommandBuffer);
	}

	// 2. Render only entities having Vertex buffers only
	for (auto gameEntity : ecs::IterateEntitiesWithAll<RenderComponent, VertexBufferComponent, NotIndexBufferComponent>())
	{
		const RenderComponent& renderComponent = ecs::ComponentManager::GetComponent<RenderComponent>(gameEntity);

		VertexBufferComponent& vertexBufferComponent = ecs::ComponentManager::GetComponent<VertexBufferComponent>(gameEntity);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
			m_pipelineLayout,
			0,
			1,
			&_frameInfo.GlobalDescriptorSet,
			1,
			&renderComponent.DynamicOffset
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
