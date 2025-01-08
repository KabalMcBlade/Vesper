#include "opaque_render_system.h"

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

#include "Backend/swap_chain.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/pipeline_components.h"

#include "Utility/logger.h"


#define MATERIAL_DIFFUSE_BINDING 0
#define MATERIAL_SPECULAR_BINDING 1
#define MATERIAL_AMBIENT_BINDING 2
#define MATERIAL_NORMAL_BINDING 3
#define MATERIAL_UNIFORM_BINDING 4


VESPERENGINE_NAMESPACE_BEGIN

// TEST PUSH CONSTAT!
struct ColorTintPushConstantData
{
	glm::vec3 ColorTint{ 1.0f };
};

OpaqueRenderSystem::OpaqueRenderSystem(VesperApp& _app, Device& _device, DescriptorPool& _globalDescriptorPool, VkRenderPass _renderPass,
	VkDescriptorSetLayout _globalDescriptorSetLayout,
	VkDescriptorSetLayout _groupDescriptorSetLayout,
	uint32 _alignedSizeUBO)
	: CoreRenderSystem{ _device }
	, m_app(_app)
	, m_globalDescriptorPool(_globalDescriptorPool)
{
	m_app.GetComponentManager().RegisterComponent<ColorTintPushConstantData>();

	m_materialSetLayout = DescriptorSetLayout::Builder(_device)
		.AddBinding(MATERIAL_DIFFUSE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(MATERIAL_SPECULAR_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(MATERIAL_AMBIENT_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(MATERIAL_NORMAL_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(MATERIAL_UNIFORM_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build();

	// TEST PUSH CONSTANT!
	// REMEMBER THAT TO TEST THIS WITH THIS PIPELINE AND LAYOUT ALSO THE MASTER RENDERER NEED TO HAVE!
	// SINCE IS A TEST, REMEMBER TO REMOVE BOTH HERE AND IN MASTER RENDERER EVENTUALLY!
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(ColorTintPushConstantData);

	m_pushConstants.push_back(pushConstantRange);

	CreatePipelineLayout(std::vector<VkDescriptorSetLayout>
			{ _globalDescriptorSetLayout, _groupDescriptorSetLayout, m_materialSetLayout->GetDescriptorSetLayout() }
		);
	CreatePipeline(_renderPass);

	m_alignedSizeUBO = _alignedSizeUBO;
}

OpaqueRenderSystem::~OpaqueRenderSystem()
{
	m_app.GetComponentManager().UnregisterComponent<ColorTintPushConstantData>();
}

void OpaqueRenderSystem::RegisterEntity(ecs::Entity _entity) const
{
	// just to ensure it has, but the RenderComponent should be already present
	if (!m_app.GetComponentManager().HasComponents<RenderComponent>(_entity))
	{
		m_app.GetComponentManager().AddComponent<RenderComponent>(_entity);
	}

	// Add opaque pipeline component if missing
	if (!m_app.GetComponentManager().HasComponents<PipelineOpaqueComponent>(_entity))
	{
		m_app.GetComponentManager().AddComponent<PipelineOpaqueComponent>(_entity);
	}

	// Add push constant component if missing
	if (!m_app.GetComponentManager().HasComponents<ColorTintPushConstantData>(_entity))
	{
		m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(_entity);
	}

	// don't add a check here, so we avoid to add twice the same entity, it will trigger an assert
	m_app.GetComponentManager().AddComponent<DynamicOffsetComponent>(_entity);

	DynamicOffsetComponent& dynamicUniformBufferComponent = m_app.GetComponentManager().GetComponent<DynamicOffsetComponent>(_entity);
	dynamicUniformBufferComponent.DynamicOffsetIndex = m_internalCounter;
	dynamicUniformBufferComponent.DynamicOffset = m_internalCounter * m_alignedSizeUBO;

	++m_internalCounter;

	// Material binding
	if (m_app.GetComponentManager().HasComponents<PhongMaterialComponent>(_entity))
	{
		PhongMaterialComponent& materialComponent = m_app.GetComponentManager().GetComponent<PhongMaterialComponent>(_entity);
		materialComponent.BoundDescriptorSet.resize(SwapChain::kMaxFramesInFlight);

		for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
		{
			DescriptorWriter(*m_materialSetLayout, m_globalDescriptorPool)
				.WriteImage(0, &materialComponent.DiffuseImageInfo)
				.WriteImage(1, &materialComponent.SpecularImageInfo)
				.WriteImage(2, &materialComponent.AmbientImageInfo)
				.WriteImage(3, &materialComponent.NormalImageInfo)
				.WriteBuffer(4, &materialComponent.UniformBufferInfo)
				.Build(materialComponent.BoundDescriptorSet[i]);
		}
	}
	else
	{
		LOG(Logger::ERROR, "Expected Material, but is missing!");
	}
}

void OpaqueRenderSystem::UnregisterEntity(ecs::Entity _entity) const
{
	if (m_app.GetComponentManager().HasComponents<PipelineOpaqueComponent, ColorTintPushConstantData>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<PipelineOpaqueComponent>(_entity);
		m_app.GetComponentManager().RemoveComponent<ColorTintPushConstantData>(_entity);
	}
}

void OpaqueRenderSystem::UnregisterEntities() const
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto entity : ecs::IterateEntitiesWithAll<PipelineOpaqueComponent, ColorTintPushConstantData>(entityManager, componentManager))
	{
		UnregisterEntity(entity);
	}
}

void OpaqueRenderSystem::Update(const FrameInfo& _frameInfo)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto gameEntity : ecs::IterateEntitiesWithAll<RenderComponent, TransformComponent, PipelineOpaqueComponent>(entityManager, componentManager))
	{
		TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);
		RenderComponent& renderComponent = componentManager.GetComponent<RenderComponent>(gameEntity);

		renderComponent.ModelMatrix = glm::translate(glm::mat4{ 1.0f }, transformComponent.Position);
		renderComponent.ModelMatrix = renderComponent.ModelMatrix * glm::toMat4(transformComponent.Rotation);
		renderComponent.ModelMatrix = glm::scale(renderComponent.ModelMatrix, transformComponent.Scale);

		ColorTintPushConstantData& pushComponent = componentManager.GetComponent<ColorTintPushConstantData>(gameEntity);

		// Use a sine wave to create smooth transitions for R, G, and B
		static const float speed = 1.0f;
		static float frameTimeUpdated = 0.0f;
		const float r = 0.5f * (std::sin(speed * frameTimeUpdated) + 1.0f); // Oscillates between 0 and 1
		const float g = 0.5f * (std::sin(speed * frameTimeUpdated + glm::pi<float>() / 3.0f) + 1.0f); // Offset by 120 degrees
		const float b = 0.5f * (std::sin(speed * frameTimeUpdated + 2.0f * glm::pi<float>() / 3.0f) + 1.0f); // Offset by 240 degrees
		frameTimeUpdated += _frameInfo.FrameTime;
		pushComponent.ColorTint = glm::vec3(r, g, b);
	}
}

void OpaqueRenderSystem::Render(const FrameInfo& _frameInfo)
{
	// this bind only the opaque pipeline
	m_opaquePipeline->Bind(_frameInfo.CommandBuffer);

	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	// 1. Render whatever has vertex buffers and index buffer
	auto entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<PhongMaterialComponent, PipelineOpaqueComponent, DynamicOffsetComponent, VertexBufferComponent, IndexBufferComponent, ColorTintPushConstantData>(entityManager, componentManager, &PhongMaterialComponent::Index);

	for (const auto& [key, entities] : entitiesGroupedAndCollected)
	{
		// From the first one and only, we can the material and we bind it.
		const PhongMaterialComponent& phongMaterialComponent = componentManager.GetComponent<PhongMaterialComponent>(entities[0]);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			2,
			1,
			&phongMaterialComponent.BoundDescriptorSet[_frameInfo.FrameIndex],
			0,
			nullptr
		);

		for (const auto& entityCollected : entities)
		{
			const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(entityCollected);

			const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(entityCollected);
			const IndexBufferComponent& indexBufferComponent = componentManager.GetComponent<IndexBufferComponent>(entityCollected);
			const ColorTintPushConstantData& pushComponent = componentManager.GetComponent<ColorTintPushConstantData>(entityCollected);

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

			PushConstants(_frameInfo.CommandBuffer, 0, &pushComponent);
			Bind(vertexBufferComponent, indexBufferComponent, _frameInfo.CommandBuffer);
			Draw(indexBufferComponent, _frameInfo.CommandBuffer);
		}
	}

	entitiesGroupedAndCollected.clear();



	// 2. Render only entities having Vertex buffers only
	entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<PhongMaterialComponent, DynamicOffsetComponent, VertexBufferComponent, NotIndexBufferComponent, ColorTintPushConstantData>(entityManager, componentManager, &PhongMaterialComponent::Index);

	for (const auto& [key, entities] : entitiesGroupedAndCollected)
	{
		// From the first one and only, we can the material and we bind it.
		const PhongMaterialComponent& phongMaterialComponent = componentManager.GetComponent<PhongMaterialComponent>(entities[0]);

		vkCmdBindDescriptorSets(
			_frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			2,
			1,
			&phongMaterialComponent.BoundDescriptorSet[_frameInfo.FrameIndex],
			0,
			nullptr
		);

		for (const auto& entityCollected : entities)
		{
			const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(entityCollected);
			const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(entityCollected);
			const ColorTintPushConstantData& pushComponent = componentManager.GetComponent<ColorTintPushConstantData>(entityCollected);

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

			PushConstants(_frameInfo.CommandBuffer, 0, &pushComponent);
			Bind(vertexBufferComponent, _frameInfo.CommandBuffer);
			Draw(vertexBufferComponent, _frameInfo.CommandBuffer);
		}
	}

	entitiesGroupedAndCollected.clear();
}

void OpaqueRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
	assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};

	Pipeline::OpaquePipelineConfiguration(pipelineConfig);

	pipelineConfig.RenderPass = _renderPass;
	pipelineConfig.PipelineLayout = m_pipelineLayout;

	ShaderInfo vertexShader(
		m_app.GetConfig().ShadersPath + "default_shader.vert.spv",
		ShaderType::Vertex
	);

	ShaderInfo fragmentShader(
		m_app.GetConfig().ShadersPath + "opaque_shader.frag.spv",
		ShaderType::Fragment
	);
	fragmentShader.AddSpecializationConstant(0, 2.0f);

	m_opaquePipeline = std::make_unique<Pipeline>(
		m_device,
		std::vector{
			vertexShader,
			fragmentShader,
		},
		pipelineConfig
		);
}

VESPERENGINE_NAMESPACE_END
