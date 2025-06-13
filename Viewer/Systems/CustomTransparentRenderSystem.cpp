// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\CustomTransparentRenderSystem.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "CustomTransparentRenderSystem.h"

#include "../Components/PushConstants.h"

#include <cstdlib>

VESPERENGINE_USING_NAMESPACE

CustomTransparentRenderSystem::CustomTransparentRenderSystem(VesperApp& app, Device& device, Renderer& renderer,
    VkDescriptorSetLayout globalDescriptorSetLayout,
    VkDescriptorSetLayout entityDescriptorSetLayout,
    VkDescriptorSetLayout bindlessBindingDescriptorSetLayout)
    : TransparentRenderSystem(app, device, renderer, globalDescriptorSetLayout,
        entityDescriptorSetLayout, bindlessBindingDescriptorSetLayout)
{
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();
    if (!componentManager.IsComponentRegistered<ColorTintPushConstantData>())
    {
        componentManager.RegisterComponent<ColorTintPushConstantData>();
    }
}

CustomTransparentRenderSystem::~CustomTransparentRenderSystem()
{
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();
    if (componentManager.IsComponentRegistered<ColorTintPushConstantData>())
    {
        componentManager.UnregisterComponent<ColorTintPushConstantData>();
    }
}

void CustomTransparentRenderSystem::PerEntityUpdate(const FrameInfo& _frameInfo, ecs::ComponentManager& _componentManager, const ecs::Entity& _entity)
{
    if (_componentManager.HasComponents<ColorTintPushConstantData>(_entity))
    {
        // Use a sine wave to create smooth transitions for R, G, and B
        static const float speed = 1.0f;
        static float frameTimeUpdated = 0.0f;
        const float r = 0.5f * (std::sin(speed * frameTimeUpdated) + 1.0f); // Oscillates between 0 and 1
        const float g = 0.5f * (std::sin(speed * frameTimeUpdated + glm::pi<float>() / 3.0f) + 1.0f); // Offset by 120 degrees
        const float b = 0.5f * (std::sin(speed * frameTimeUpdated + 2.0f * glm::pi<float>() / 3.0f) + 1.0f); // Offset by 240 degrees
        frameTimeUpdated += _frameInfo.FrameTime;
        
        ColorTintPushConstantData& pushComponent = _componentManager.GetComponent<ColorTintPushConstantData>(_entity);
        pushComponent.ColorTint = glm::vec3(r, g, b);
    }
}

void CustomTransparentRenderSystem::PerEntityRender(const FrameInfo& _frameInfo, ecs::ComponentManager& _componentManager, const ecs::Entity& _entity)
{
    if (_componentManager.HasComponents<ColorTintPushConstantData>(_entity))
    {
        ColorTintPushConstantData& pushComponent = _componentManager.GetComponent<ColorTintPushConstantData>(_entity);
        // it works also passing 128 bytes (the default function) but is pointless passing all those bytes for nothing!
        PushConstants(_frameInfo.CommandBuffer, 0, 0, sizeof(pushComponent), &pushComponent);
    }
}

void CustomTransparentRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
    assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::TransparentPipelineConfiguration(pipelineConfig);
    pipelineConfig.RenderPass = renderPass;
    pipelineConfig.PipelineLayout = m_pipelineLayout;

    const std::string vertexShaderFilepath = m_device.IsBindlessResourcesSupported()
        ? m_app.GetConfig().ShadersPath + "tint_phong_shader_bindless1.vert.spv"
        : m_app.GetConfig().ShadersPath + "tint_phong_shader_bindless0.vert.spv";

    ShaderInfo vertexShader(vertexShaderFilepath, ShaderType::Vertex);

    const std::string fragmentShaderFilepath = m_device.IsBindlessResourcesSupported()
        ? m_app.GetConfig().ShadersPath + "tint_phong_shader_bindless1.frag.spv"
        : m_app.GetConfig().ShadersPath + "tint_phong_shader_bindless0.frag.spv";

    ShaderInfo fragmentShader(fragmentShaderFilepath, ShaderType::Fragment);
    fragmentShader.AddSpecializationConstant(0, 2.0f);

    m_transparentPipeline = std::make_unique<Pipeline>(
        m_device,
        std::vector{ vertexShader, fragmentShader },
        pipelineConfig);
}
