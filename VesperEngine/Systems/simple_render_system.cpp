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

struct SimplePushConstantData
{
	glm::mat4 ModelMatrix{ 1.0f };
	glm::mat4 NormalModelMatrix{ 1.0f };
};

SimpleRenderSystem::SimpleRenderSystem(Device& _device, VkRenderPass _renderPass, VkDescriptorSetLayout _globalDescriptorSetLayout)
	: BaseRenderSystem {_device }
{
	// TEST: Push Constant
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	m_pushConstants.push_back(pushConstantRange);

	CreatePipelineLayout(std::vector<VkDescriptorSetLayout>{ _globalDescriptorSetLayout });
	CreatePipeline(_renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem()
{
	vkDestroyPipelineLayout(m_device.GetDevice(), m_pipelineLayout, nullptr);
}

void SimpleRenderSystem::RenderFrame(FrameInfo& _frameInfo)
{
	// Iterate all the camera and update the objects transform based on projection view
	for (auto camera : ecs::IterateEntitiesWithAll<GlobalUBO>())
	{
		// NOTE: We use the Global UBO to get from the camera, since we know it has
		const GlobalUBO& globalUBO = ecs::ComponentManager::GetComponent<GlobalUBO>(camera);

		// 1. Render whatever has vertex buffers and index buffer
		for (auto gameEntity : ecs::IterateEntitiesWithAll<TransformComponent, VertexBufferComponent, IndexBufferComponent>())
		{
			TransformEntity(_frameInfo.CommandBUffer, gameEntity, globalUBO.ProjectionView);

			VertexBufferComponent& vertexBufferComponent = ecs::ComponentManager::GetComponent<VertexBufferComponent>(gameEntity);
			IndexBufferComponent& indexBufferComponent = ecs::ComponentManager::GetComponent<IndexBufferComponent>(gameEntity);

			Bind(vertexBufferComponent, indexBufferComponent, _frameInfo.CommandBUffer);
			Draw(indexBufferComponent, _frameInfo.CommandBUffer);
		}

		// 2. Render only entities having Vertex buffers only
		for (auto gameEntity : ecs::IterateEntitiesWithAll<TransformComponent, VertexBufferComponent, NotIndexBufferComponent>())
		{
			TransformEntity(_frameInfo.CommandBUffer, gameEntity, globalUBO.ProjectionView);

			VertexBufferComponent& vertexBufferComponent = ecs::ComponentManager::GetComponent<VertexBufferComponent>(gameEntity);

			Bind(vertexBufferComponent, _frameInfo.CommandBUffer);
			Draw(vertexBufferComponent, _frameInfo.CommandBUffer);
		}
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

void SimpleRenderSystem::TransformEntity(VkCommandBuffer _commandBuffer, ecs::Entity _entity, glm::mat4 _projectionView)
{
	TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(_entity);

	auto modelMatrix = glm::translate(glm::mat4{ 1.0f }, transformComponent.Position);
	modelMatrix = modelMatrix * glm::toMat4(transformComponent.Rotation);
	modelMatrix = glm::scale(modelMatrix, transformComponent.Scale);

	SimplePushConstantData push{};
	push.ModelMatrix = modelMatrix;
	push.NormalModelMatrix = glm::transpose(glm::inverse(modelMatrix));

	vkCmdPushConstants(_commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
}

VESPERENGINE_NAMESPACE_END
