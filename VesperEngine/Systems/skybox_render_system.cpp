// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\skybox_render_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/skybox_render_system.h"

#include "Backend/pipeline.h"
#include "Backend/renderer.h"
#include "Backend/frame_info.h"
#include "Backend/descriptors.h"
#include "Backend/device.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/camera_components.h"
#include "Components/pipeline_components.h"

#include "App/vesper_app.h"
#include "App/config.h"

#include "ECS/ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

struct SkyboxPushConstant
{
    glm::mat4 ViewProjection{1.f};
};

SkyboxRenderSystem::SkyboxRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
    VkDescriptorSetLayout _globalDescriptorSetLayout,
    VkDescriptorSetLayout _bindlessDescriptorSetLayout)
    : BaseRenderSystem(_device)
    , m_app(_app)
    , m_renderer(_renderer)
{
    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_ALL;
    range.offset = 0;
    range.size = VESPERENGINE_PUSHCONSTANT_DEFAULTRANGE; //sizeof(SkyboxPushConstant);
    m_pushConstants.push_back(range);

    m_setLayout = DescriptorSetLayout::Builder(_device)
        .AddBinding(kCubemapBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();

    if (_device.IsBindlessResourcesSupported())
    {
        m_skyboxSetIndex = 2;
        CreatePipelineLayout({ _globalDescriptorSetLayout, _bindlessDescriptorSetLayout, m_setLayout->GetDescriptorSetLayout() });
    }
    else
    {
        m_skyboxSetIndex = 1;
        CreatePipelineLayout({ _globalDescriptorSetLayout, m_setLayout->GetDescriptorSetLayout() });
    }
}

void SkyboxRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
    PipelineConfigInfo config{};
    Pipeline::SkyboxPipelineConfig(config);
    config.RenderPass = _renderPass;
    config.PipelineLayout = m_pipelineLayout;

    ShaderInfo vert(m_app.GetConfig().ShadersPath + "cubemap_shader.vert.spv", ShaderType::Vertex);

    const std::string fragmentShaderFilepath = m_device.IsBindlessResourcesSupported()
        ? m_app.GetConfig().ShadersPath + "skybox_shader_bindless1.frag.spv"
        : m_app.GetConfig().ShadersPath + "skybox_shader_bindless0.frag.spv";
    ShaderInfo frag(fragmentShaderFilepath, ShaderType::Fragment);

    m_pipeline = std::make_unique<Pipeline>(m_device, std::vector{ vert, frag }, config);
}

void SkyboxRenderSystem::MaterialBinding()
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto entity : ecs::IterateEntitiesWithAll<PipelineSkyboxComponent, SkyboxMaterialComponent>(entityManager, componentManager))
    {
        SkyboxMaterialComponent& sb = componentManager.GetComponent<SkyboxMaterialComponent>(entity);
        sb.BoundDescriptorSet.resize(SwapChain::kMaxFramesInFlight);
        for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
        {
            DescriptorWriter(*m_setLayout, *m_renderer.GetDescriptorPool())
                .WriteImage(kCubemapBindingIndex, &sb.ImageInfo)
                .Build(sb.BoundDescriptorSet[i]);
        }
    }
}

void SkyboxRenderSystem::Update(const FrameInfo& _frameInfo)
{
    (void)_frameInfo;
}

void SkyboxRenderSystem::Render(const FrameInfo& _frameInfo)
{
    m_pipeline->Bind(_frameInfo.CommandBuffer);

    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    // Get active camera data
    std::vector<ecs::Entity> cameras = ecs::EntityCollector::CollectEntitiesWithAll<CameraActive, CameraComponent, CameraTransformComponent>(entityManager, componentManager);
    if (cameras.empty())
    {
        return;
    }

    CameraComponent camera = componentManager.GetComponent<CameraComponent>(cameras[0]);

    glm::mat4 view = glm::mat4(glm::mat3(camera.ViewMatrix));
    SkyboxPushConstant push{};
    push.ViewProjection = camera.ProjectionMatrix * view;

    for (auto entity : ecs::IterateEntitiesWithAll<PipelineSkyboxComponent, VertexBufferComponent, SkyboxMaterialComponent, VisibilityComponent>(entityManager, componentManager))
    {
        const VertexBufferComponent& vertex = componentManager.GetComponent<VertexBufferComponent>(entity);
        const IndexBufferComponent& index = componentManager.GetComponent<IndexBufferComponent>(entity);
        SkyboxMaterialComponent& sb = componentManager.GetComponent<SkyboxMaterialComponent>(entity);

        vkCmdBindDescriptorSets(
            _frameInfo.CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            m_skyboxSetIndex,
            1,
            &sb.BoundDescriptorSet[_frameInfo.FrameIndex],
            0,
            nullptr);

        PushConstants(_frameInfo.CommandBuffer, 0, 0, sizeof(SkyboxPushConstant), &push);
        Bind(vertex, index, _frameInfo.CommandBuffer);
        Draw(index, _frameInfo.CommandBuffer);
    }
}

void SkyboxRenderSystem::Cleanup()
{
}

VESPERENGINE_NAMESPACE_END
