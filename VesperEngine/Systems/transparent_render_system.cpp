#include "Systems/transparent_render_system.h"
#include "Systems/render_subsystem.h"
#include "Systems/color_tint_system.h"

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


VESPERENGINE_NAMESPACE_BEGIN

TransparentRenderSystem::TransparentRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
        VkDescriptorSetLayout _globalDescriptorSetLayout,
        VkDescriptorSetLayout _entityDescriptorSetLayout,
        VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout,
        const std::vector<RenderSubsystem*>& _subsystems)
        : BaseRenderSystem{ _device }
        , m_app(_app)
        , m_renderer(_renderer)
{
    for (RenderSubsystem* subsystem : _subsystems)
    {
        AddRenderSubsystem(subsystem);
    }
    if (m_renderSubsystems.empty())
    {
        m_defaultColorTintSubsystem = std::make_unique<DefaultColorTintSubsystem>();
        AddRenderSubsystem(m_defaultColorTintSubsystem.get());
    }
    m_buffer = std::make_unique<Buffer>(m_device);

    if (m_device.IsBindlessResourcesSupported())
    {
        m_materialSetLayout = DescriptorSetLayout::Builder(_device)
                .AddBinding(kPhongUniformBufferOnlyBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .Build();
    }
    else
    {
        m_materialSetLayout = DescriptorSetLayout::Builder(_device)
                .AddBinding(kPhongAmbientTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .AddBinding(kPhongDiffuseTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .AddBinding(kPhongSpecularTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .AddBinding(kPhongNormalTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .AddBinding(kPhongAlphaTextureBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .AddBinding(kPhongUniformBufferBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
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

    CreatePipeline(m_renderer.GetSwapChainRenderPass());
}

void TransparentRenderSystem::MaterialBinding()
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto gameEntity : ecs::IterateEntitiesWithAll<PipelineTransparentComponent, PhongMaterialComponent>(entityManager, componentManager))
    {
        PhongMaterialComponent& materialComponent = m_app.GetComponentManager().GetComponent<PhongMaterialComponent>(gameEntity);
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

                PhongBindlessMaterialIndexUBO phongIndexUBO;
                phongIndexUBO.MaterialIndex = materialComponent.Index;

                currentBuffer.MappedMemory = &phongIndexUBO;
                m_buffer->WriteToBuffer(currentBuffer);

                VkDescriptorBufferInfo bufferInfo;
                bufferInfo.buffer = currentBuffer.Buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = currentBuffer.AlignedSize;

                DescriptorWriter(*m_materialSetLayout, *m_renderer.GetDescriptorPool())
                        .WriteBuffer(kPhongUniformBufferOnlyBindingIndex, &bufferInfo)
                        .Build(materialComponent.BoundDescriptorSet[i]);
            }
        }
        else
        {
            for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
            {
                DescriptorWriter(*m_materialSetLayout, *m_renderer.GetDescriptorPool())
                        .WriteImage(kPhongAmbientTextureBindingIndex, &materialComponent.AmbientImageInfo)
                        .WriteImage(kPhongDiffuseTextureBindingIndex, &materialComponent.DiffuseImageInfo)
                        .WriteImage(kPhongSpecularTextureBindingIndex, &materialComponent.SpecularImageInfo)
                        .WriteImage(kPhongNormalTextureBindingIndex, &materialComponent.NormalImageInfo)
                        .WriteImage(kPhongAlphaTextureBindingIndex, &materialComponent.AlphaImageInfo)
                        .WriteBuffer(kPhongUniformBufferBindingIndex, &materialComponent.UniformBufferInfo)
                        .Build(materialComponent.BoundDescriptorSet[i]);
            }
        }
    }
}

void TransparentRenderSystem::Update(const FrameInfo& _frameInfo)
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto gameEntity : ecs::IterateEntitiesWithAll<PipelineTransparentComponent, RenderComponent, TransformComponent>(entityManager, componentManager))
    {
        TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);
        RenderComponent& renderComponent = componentManager.GetComponent<RenderComponent>(gameEntity);

        renderComponent.ModelMatrix = glm::translate(glm::mat4{ 1.0f }, transformComponent.Position);
        renderComponent.ModelMatrix = renderComponent.ModelMatrix * glm::toMat4(transformComponent.Rotation);
        renderComponent.ModelMatrix = glm::scale(renderComponent.ModelMatrix, transformComponent.Scale);
    }
}

void TransparentRenderSystem::Render(const FrameInfo& _frameInfo)
{
    m_transparentPipeline->Bind(_frameInfo.CommandBuffer);

    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    auto entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<PhongMaterialComponent, PipelineTransparentComponent, DynamicOffsetComponent, VertexBufferComponent, IndexBufferComponent>(entityManager, componentManager, &PhongMaterialComponent::Index);

    for (const auto& [key, entities] : entitiesGroupedAndCollected)
    {
        const PhongMaterialComponent& phongMaterialComponent = componentManager.GetComponent<PhongMaterialComponent>(entities[0]);

        vkCmdBindDescriptorSets(
                _frameInfo.CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                m_materialSetIndex,
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

            for (RenderSubsystem* subsystem : m_renderSubsystems)
            {
                subsystem->Execute(_frameInfo.CommandBuffer, m_pipelineLayout, entityCollected);
            }
            Bind(vertexBufferComponent, indexBufferComponent, _frameInfo.CommandBuffer);
            Draw(indexBufferComponent, _frameInfo.CommandBuffer);
        }
    }

    entitiesGroupedAndCollected.clear();

    entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<PhongMaterialComponent, PipelineTransparentComponent, DynamicOffsetComponent, VertexBufferComponent, NotIndexBufferComponent>(entityManager, componentManager, &PhongMaterialComponent::Index);

    for (const auto& [key, entities] : entitiesGroupedAndCollected)
    {
        const PhongMaterialComponent& phongMaterialComponent = componentManager.GetComponent<PhongMaterialComponent>(entities[0]);

        vkCmdBindDescriptorSets(
                _frameInfo.CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                m_materialSetIndex,
                1,
                &phongMaterialComponent.BoundDescriptorSet[_frameInfo.FrameIndex],
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

            for (RenderSubsystem* subsystem : m_renderSubsystems)
            {
                subsystem->Execute(_frameInfo.CommandBuffer, m_pipelineLayout, entityCollected);
            }
            Bind(vertexBufferComponent, _frameInfo.CommandBuffer);
            Draw(vertexBufferComponent, _frameInfo.CommandBuffer);
        }
    }

    entitiesGroupedAndCollected.clear();
}

void TransparentRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
    assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};

    Pipeline::TransparentPipelineConfiguration(pipelineConfig);

    pipelineConfig.RenderPass = _renderPass;
    pipelineConfig.PipelineLayout = m_pipelineLayout;

    const std::string vertexShaderFilepath = m_device.IsBindlessResourcesSupported()
            ? m_app.GetConfig().ShadersPath + "phong_shader_bindless1.vert.spv"
            : m_app.GetConfig().ShadersPath + "phong_shader_bindless0.vert.spv";

    ShaderInfo vertexShader(
            vertexShaderFilepath,
            ShaderType::Vertex
    );

    const std::string fragmentShaderFilepath = m_device.IsBindlessResourcesSupported()
            ? m_app.GetConfig().ShadersPath + "phong_shader_bindless1.frag.spv"
            : m_app.GetConfig().ShadersPath + "phong_shader_bindless0.frag.spv";

    ShaderInfo fragmentShader(
            fragmentShaderFilepath,
            ShaderType::Fragment
    );

    fragmentShader.AddSpecializationConstant(0, 2.0f);

    m_transparentPipeline = std::make_unique<Pipeline>(
            m_device,
            std::vector{
                    vertexShader,
                    fragmentShader,
            },
            pipelineConfig
            );
}

void TransparentRenderSystem::Cleanup()
{
    for (int32 i = 0; i < m_bindlessBindingMaterialIndexUbos.size(); ++i)
    {
        m_buffer->Destroy(m_bindlessBindingMaterialIndexUbos[i]);
    }
}

VESPERENGINE_NAMESPACE_END
