#include "push_constant_render_system.h"

#include <array>
#include <stdexcept>
#include <iostream>

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "gtx/quaternion.hpp"

#include "Components/graphics_components.h"
#include "Components/object_components.h"

#include "App/vesper_app.h"


VESPERENGINE_NAMESPACE_BEGIN

struct SimplePushConstantData
{
	glm::mat4 ModelMatrix{ 1.0f };
};

PushConstantRenderSystem::PushConstantRenderSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass, VkDescriptorSetLayout _globalDescriptorSetLayout)
	: m_app(_app)
	, BaseRenderSystem {_device }
{
	m_app.GetComponentManager().RegisterComponent<SimplePushConstantData>();

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	m_pushConstants.push_back(pushConstantRange);

	CreatePipelineLayout(std::vector<VkDescriptorSetLayout>{ _globalDescriptorSetLayout });
	CreatePipeline(_renderPass);
}

PushConstantRenderSystem::~PushConstantRenderSystem()
{
	m_app.GetComponentManager().UnregisterComponent<SimplePushConstantData>();
}

void PushConstantRenderSystem::RegisterEntity(ecs::Entity _entity) const
{
	m_app.GetComponentManager().AddComponent<SimplePushConstantData>(_entity);
}

void PushConstantRenderSystem::UnregisterEntity(ecs::Entity _entity) const
{
	if (m_app.GetComponentManager().HasComponents<SimplePushConstantData>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<SimplePushConstantData>(_entity);
	}
}

void PushConstantRenderSystem::UnregisterEntities() const
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto entity : ecs::IterateEntitiesWithAll<SimplePushConstantData>(entityManager, componentManager))
	{
		UnregisterEntity(entity);
	}
}

void PushConstantRenderSystem::UpdateFrame(const FrameInfo& _frameInfo)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto gameEntity : ecs::IterateEntitiesWithAll<RenderComponent, TransformComponent, SimplePushConstantData>(entityManager, componentManager))
	{
		TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);
		RenderComponent& renderComponent = componentManager.GetComponent<RenderComponent>(gameEntity);

		renderComponent.ModelMatrix = glm::translate(glm::mat4{ 1.0f }, transformComponent.Position);
		renderComponent.ModelMatrix = renderComponent.ModelMatrix * glm::toMat4(transformComponent.Rotation);
		renderComponent.ModelMatrix = glm::scale(renderComponent.ModelMatrix, transformComponent.Scale);

		SimplePushConstantData& push = componentManager.GetComponent<SimplePushConstantData>(gameEntity);
		push.ModelMatrix = renderComponent.ModelMatrix;
	}
}

void PushConstantRenderSystem::RenderFrame(const FrameInfo& _frameInfo)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	// 1. Render whatever has vertex buffers and index buffer
	for (auto gameEntity : ecs::IterateEntitiesWithAll<VertexBufferComponent, IndexBufferComponent, SimplePushConstantData>(entityManager, componentManager))
	{
		const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(gameEntity);
		const IndexBufferComponent& indexBufferComponent = componentManager.GetComponent<IndexBufferComponent>(gameEntity);
		const SimplePushConstantData& push = componentManager.GetComponent<SimplePushConstantData>(gameEntity);
		
		const RenderComponent& renderComponent = componentManager.GetComponent<RenderComponent>(gameEntity);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
			m_pipelineLayout,
			0,
			1,
			&_frameInfo.GlobalDescriptorSet,
			0,
			nullptr
		);

		PushConstants(_frameInfo.CommandBuffer, 0, &push);
		Bind(vertexBufferComponent, indexBufferComponent, _frameInfo.CommandBuffer);
		Draw(indexBufferComponent, _frameInfo.CommandBuffer);
	}

	// 2. Render only entities having Vertex buffers only
	for (auto gameEntity : ecs::IterateEntitiesWithAll<VertexBufferComponent, NotIndexBufferComponent, SimplePushConstantData>(entityManager, componentManager))
	{
		const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(gameEntity);
		const SimplePushConstantData& push = componentManager.GetComponent<SimplePushConstantData>(gameEntity);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
			m_pipelineLayout,
			0,
			1,
			&_frameInfo.GlobalDescriptorSet,
			1,
			0
		);

		PushConstants(_frameInfo.CommandBuffer, 0, &push);
		Bind(vertexBufferComponent, _frameInfo.CommandBuffer);
		Draw(vertexBufferComponent, _frameInfo.CommandBuffer);
	}
}

void PushConstantRenderSystem::SetupPipeline(PipelineConfigInfo& _pipelineConfig)
{
	m_pipeline = std::make_unique<Pipeline>(
		m_device,
		std::vector{ ShaderInfo{ m_app.GetConfig().ShadersPath + "push_constant_shader.vert.spv", ShaderType::Vertex}, ShaderInfo{m_app.GetConfig().ShadersPath + "push_constant_shader.frag.spv", ShaderType::Fragment}, },
		_pipelineConfig
		);
}

VESPERENGINE_NAMESPACE_END
