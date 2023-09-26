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
	glm::mat4 Transform{ 1.0f };
	glm::vec4 Color;
};

SimpleRenderSystem::SimpleRenderSystem(Device& _device, VkRenderPass _renderPass)
	: CoreRenderSystem {_device }
{
	CreatePipelineLayout();
	CreatePipeline(_renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem()
{
	vkDestroyPipelineLayout(m_device.GetDevice(), m_pipelineLayout, nullptr);
}

void SimpleRenderSystem::CreatePipelineLayout()
{
	// TEST: Push Constant
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);


	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	// This is used to pass other than vertex data to the vertex and fragment shaders.
	// This can include textures and uniform buffer objects (UBO)
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;

	// Push constants are a efficient way to send a small amount of data to the shader programs
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(m_device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void SimpleRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
	assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	Pipeline::DefaultPipelineConfiguration(pipelineConfig);

	// The render pass describes the structure and format of the frame buffer objects (FBOs) and their attachments
	// Right noe we have in location 0 the color buffer and in location 1 the depth buffer, check SwapChain::CreateRenderPass() -> attachments
	// So we need to inform the shaders about these locations. If the render pass change, shaders must reflect it.
	pipelineConfig.RenderPass = _renderPass;
	pipelineConfig.PipelineLayout = m_pipelineLayout;
	m_pipeline = std::make_unique<Pipeline>(
		m_device,
		"Assets/Shaders/simple_shader.vert.spv",
		"Assets/Shaders/simple_shader.frag.spv",
		pipelineConfig
		);
}

void SimpleRenderSystem::RenderGameEntities(VkCommandBuffer _commandBuffer)
{
	m_pipeline->Bind(_commandBuffer);

	// Iterate all the camera and update the objects transform based on projection view
	for (auto camera : ecs::IterateEntitiesWithAll<CameraComponent>())
	{
		const CameraComponent& cameraComponent = ecs::ComponentManager::GetComponent<CameraComponent>(camera);
		
		auto projectionView = cameraComponent.ProjectionMatrix * cameraComponent.ViewMatrix;

		// First: update transform and push constants
		// NOTE: the camera has a special transform CameraTransformComponent, so is not collected from here
		for (auto gameEntity : ecs::IterateEntitiesWithAll<TransformComponent>())
		{
			TransformComponent& transformComponent = ecs::ComponentManager::GetComponent<TransformComponent>(gameEntity);

			auto transform = glm::translate(glm::mat4{ 1.0f }, transformComponent.Position);
			transform = transform * glm::toMat4(transformComponent.Rotation);
			transform = glm::scale(transform, transformComponent.Scale);

			SimplePushConstantData push{};
			push.Transform = projectionView * transform;
			push.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

			vkCmdPushConstants(_commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
		}
	}

	// Second: render whatever has both VertexBufferComponent and IndexBufferComponent
	for (auto gameEntity : ecs::IterateEntitiesWithAll<VertexBufferComponent, IndexBufferComponent>())
	{
		VertexBufferComponent& vertexBufferComponent = ecs::ComponentManager::GetComponent<VertexBufferComponent>(gameEntity);
		IndexBufferComponent& indexBufferComponent = ecs::ComponentManager::GetComponent<IndexBufferComponent>(gameEntity);

		Bind(vertexBufferComponent, indexBufferComponent, _commandBuffer);
		Draw(indexBufferComponent, _commandBuffer);
	}

	// honestly we should have only object with indices
	// 
// 	// Third: whatever has only VertexBufferComponent
// 	for (auto gameEntity : ecs::IterateEntitiesWithAll<VertexBufferComponent>())
// 	{
// 		VertexBufferComponent& vertexBufferComponent = ecs::ComponentManager::GetComponent<VertexBufferComponent>(gameEntity);
// 
// 		Bind(vertexBufferComponent, _commandBuffer);
// 		Draw(vertexBufferComponent, _commandBuffer);
// 	}
}

VESPERENGINE_NAMESPACE_END
