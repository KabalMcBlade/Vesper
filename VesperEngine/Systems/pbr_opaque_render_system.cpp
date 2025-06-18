// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\pbr_opaque_render_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/pbr_opaque_render_system.h"

#include "Core/glm_config.h"

#include "Backend/pipeline.h"
#include "Backend/frame_info.h"
#include "Backend/descriptors.h"
#include "Backend/renderer.h"
#include "Backend/buffer.h"
#include "Backend/swap_chain.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/pipeline_components.h"

#include "Systems/uniform_buffer.h"

#include "App/vesper_app.h"
#include "App/config.h"

#include "ECS/ECS/ecs.h"

#include <array>
#include <stdexcept>

VESPERENGINE_NAMESPACE_BEGIN

PBROpaqueRenderSystem::PBROpaqueRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
    VkDescriptorSetLayout _globalDescriptorSetLayout,
    VkDescriptorSetLayout _entityDescriptorSetLayout,
    VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout)
    : BaseRenderSystem{ _device }
    , m_app(_app)
    , m_renderer(_renderer)
{
    m_buffer = std::make_unique<Buffer>(m_device);

    VkPushConstantRange defaultRange{};
    defaultRange.stageFlags = VK_SHADER_STAGE_ALL;
    defaultRange.offset = 0;
    defaultRange.size = VESPERENGINE_PUSHCONSTANT_DEFAULTRANGE;
    m_pushConstants.push_back(defaultRange);

    if (m_device.IsBindlessResourcesSupported())
    {
        m_materialSetLayout = DescriptorSetLayout::Builder(_device)
            .AddBinding(kPBRUniformBufferOnlyBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();
    }
    else
    {
        m_materialSetLayout = DescriptorSetLayout::Builder(_device)
            .AddBinding(kPBRRoughnessTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(kPBRMetallicTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(kPBRSheenTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(kPBREmissiveTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(kPBRNormalTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(kPBRBaseColorTextureBindingIndex,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(kPBRAOTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(kPBRUniformBufferBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();
    }

    if (m_device.IsBindlessResourcesSupported())
    {
        m_entitySetIndex = 2;
        m_materialSetIndex = 3;

        CreatePipelineLayout(std::vector<VkDescriptorSetLayout>
        { _globalDescriptorSetLayout, _bindlessBindingDescriptorSetLayout, _entityDescriptorSetLayout, m_materialSetLayout->GetDescriptorSetLayout() }
        );
    }
    else
    {
        CreatePipelineLayout(std::vector<VkDescriptorSetLayout>
        { _globalDescriptorSetLayout, _entityDescriptorSetLayout, m_materialSetLayout->GetDescriptorSetLayout() }
        );
    }
}

void PBROpaqueRenderSystem::MaterialBinding()
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto gameEntity : ecs::IterateEntitiesWithAll<PipelineOpaqueComponent, PBRMaterialComponent>(entityManager, componentManager))
    {
        PBRMaterialComponent& materialComponent = m_app.GetComponentManager().GetComponent<PBRMaterialComponent>(gameEntity);
        materialComponent.BoundDescriptorSet.resize(SwapChain::kMaxFramesInFlight);

        if (m_device.IsBindlessResourcesSupported())
        {
            for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
            {
                m_bindlessBindingMaterialIndexUbos.emplace_back(m_buffer->Create<BufferComponent>(
                    sizeof(PhongBindlessMaterialIndexUBO),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
                    /*minUboAlignment*/1,
                    true
                ));

                BufferComponent& currentBuffer = m_bindlessBindingMaterialIndexUbos.back();

                PhongBindlessMaterialIndexUBO pbrIndexUBO; // same layout
                pbrIndexUBO.MaterialIndex = materialComponent.Index;

                currentBuffer.MappedMemory = &pbrIndexUBO;
                m_buffer->WriteToBuffer(currentBuffer);

                VkDescriptorBufferInfo bufferInfo;
                bufferInfo.buffer = currentBuffer.Buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = currentBuffer.AlignedSize;

                DescriptorWriter(*m_materialSetLayout, *m_renderer.GetDescriptorPool())
                    .WriteBuffer(kPBRUniformBufferOnlyBindingIndex, &bufferInfo)
                    .Build(materialComponent.BoundDescriptorSet[i]);
            }
        }
        else
        {
            for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
            {
                DescriptorWriter(*m_materialSetLayout, *m_renderer.GetDescriptorPool())
                    .WriteImage(kPBRRoughnessTextureBindingIndex, &materialComponent.RoughnessImageInfo)
                    .WriteImage(kPBRMetallicTextureBindingIndex, &materialComponent.MetallicImageInfo)
                    .WriteImage(kPBRSheenTextureBindingIndex, &materialComponent.SheenImageInfo)
                    .WriteImage(kPBREmissiveTextureBindingIndex, &materialComponent.EmissiveImageInfo)
                    .WriteImage(kPBRNormalTextureBindingIndex, &materialComponent.NormalImageInfo)
                    .WriteImage(kPBRBaseColorTextureBindingIndex, &materialComponent.BaseColorImageInfo)
                    .WriteImage(kPBRAOTextureBindingIndex, &materialComponent.AOImageInfo)
                    .WriteBuffer(kPBRUniformBufferBindingIndex, &materialComponent.UniformBufferInfo)
                    .Build(materialComponent.BoundDescriptorSet[i]);
            }
        }
    }
}

void PBROpaqueRenderSystem::Update(const FrameInfo& _frameInfo)
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto gameEntity : ecs::IterateEntitiesWithAll<UpdateComponent, TransformComponent, PipelineOpaqueComponent>(entityManager, componentManager))
    {
        TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);
        UpdateComponent& updateComponent = componentManager.GetComponent<UpdateComponent>(gameEntity);

        updateComponent.ModelMatrix = glm::translate(glm::mat4{ 1.0f }, transformComponent.Position);
        updateComponent.ModelMatrix = updateComponent.ModelMatrix * glm::toMat4(transformComponent.Rotation);
        updateComponent.ModelMatrix = glm::scale(updateComponent.ModelMatrix, transformComponent.Scale);

        PerEntityUpdate(_frameInfo, componentManager, gameEntity);
    }
}

void PBROpaqueRenderSystem::Render(const FrameInfo& _frameInfo)
{
    m_pipeline->Bind(_frameInfo.CommandBuffer);

    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    auto entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<PBRMaterialComponent, PipelineOpaqueComponent, DynamicOffsetComponent, VertexBufferComponent, IndexBufferComponent, VisibilityComponent>(entityManager, componentManager, &PBRMaterialComponent::Index);

    for (const auto& [key, entities] : entitiesGroupedAndCollected)
    {
        const PBRMaterialComponent& materialComponent = componentManager.GetComponent<PBRMaterialComponent>(entities[0]);

        vkCmdBindDescriptorSets(
            _frameInfo.CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            m_materialSetIndex,
            1,
            &materialComponent.BoundDescriptorSet[_frameInfo.FrameIndex],
            0,
            nullptr
        );

        for (const auto& entityCollected : entities)
        {
            const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(entityCollected);
            const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(entityCollected);
            const IndexBufferComponent& indexBufferComponent = componentManager.GetComponent<IndexBufferComponent>(entityCollected);

            vkCmdBindDescriptorSets(
                _frameInfo.CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                m_entitySetIndex,
                1,
                &_frameInfo.EntityDescriptorSet,
                1,
                &dynamicOffsetComponent.DynamicOffset
            );

            PerEntityRender(_frameInfo, componentManager, entityCollected);

            Bind(vertexBufferComponent, indexBufferComponent, _frameInfo.CommandBuffer);
            Draw(indexBufferComponent, _frameInfo.CommandBuffer);
        }
    }

    entitiesGroupedAndCollected.clear();

    entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<PBRMaterialComponent, PipelineOpaqueComponent, DynamicOffsetComponent, VertexBufferComponent, NotIndexBufferComponent, VisibilityComponent>(entityManager, componentManager, &PBRMaterialComponent::Index);

    for (const auto& [key, entities] : entitiesGroupedAndCollected)
    {
        const PBRMaterialComponent& materialComponent = componentManager.GetComponent<PBRMaterialComponent>(entities[0]);

        vkCmdBindDescriptorSets(
            _frameInfo.CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            m_materialSetIndex,
            1,
            &materialComponent.BoundDescriptorSet[_frameInfo.FrameIndex],
            0,
            nullptr
        );

        for (const auto& entityCollected : entities)
        {
            const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(entityCollected);
            const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(entityCollected);

            vkCmdBindDescriptorSets(
                _frameInfo.CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                m_entitySetIndex,
                1,
                &_frameInfo.EntityDescriptorSet,
                1,
                &dynamicOffsetComponent.DynamicOffset
            );

            PerEntityRender(_frameInfo, componentManager, entityCollected);

            Bind(vertexBufferComponent, _frameInfo.CommandBuffer);
            Draw(vertexBufferComponent, _frameInfo.CommandBuffer);
        }
    }

    entitiesGroupedAndCollected.clear();
}

void PBROpaqueRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
    assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};

    Pipeline::OpaquePipelineConfiguration(pipelineConfig);

    pipelineConfig.RenderPass = _renderPass;
    pipelineConfig.PipelineLayout = m_pipelineLayout;

    const std::string vertexShaderFilepath = m_device.IsBindlessResourcesSupported()
        ? m_app.GetConfig().ShadersPath + "pbr_shader_bindless1.vert.spv"
        : m_app.GetConfig().ShadersPath + "pbr_shader_bindless0.vert.spv";

    ShaderInfo vertexShader(
        vertexShaderFilepath,
        ShaderType::Vertex
    );

    const std::string fragmentShaderFilepath = m_device.IsBindlessResourcesSupported()
        ? m_app.GetConfig().ShadersPath + "pbr_shader_bindless1.frag.spv"
        : m_app.GetConfig().ShadersPath + "pbr_shader_bindless0.frag.spv";

    ShaderInfo fragmentShader(
        fragmentShaderFilepath,
        ShaderType::Fragment
    );

    fragmentShader.AddSpecializationConstant(0, 2.0f);

    m_pipeline = std::make_unique<Pipeline>(
        m_device,
        std::vector{
                vertexShader,
                fragmentShader,
        },
        pipelineConfig
        );
}

void PBROpaqueRenderSystem::Cleanup()
{
    for (int32 i = 0; i < m_bindlessBindingMaterialIndexUbos.size(); ++i)
    {
        m_buffer->Destroy(m_bindlessBindingMaterialIndexUbos[i]);
    }
}

VESPERENGINE_NAMESPACE_END
