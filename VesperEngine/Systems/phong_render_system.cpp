#include "Systems/phong_render_system.h"

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

PhongRenderSystem::PhongRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
        VkDescriptorSetLayout _globalDescriptorSetLayout,
        VkDescriptorSetLayout _entityDescriptorSetLayout,
        VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout)
    : BaseRenderSystem{ _device }
    , m_app(_app)
    , m_renderer(_renderer)
{
    m_buffer = std::make_unique<Buffer>(m_device);

    m_app.GetComponentManager().RegisterComponent<PhongRenderSystem::ColorTintPushConstantData>();

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
            .AddBinding(kPhongUniformBufferBindingIndex, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();
    }

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PhongRenderSystem::ColorTintPushConstantData);

    m_pushConstants.push_back(pushConstantRange);

    if (m_device.IsBindlessResourcesSupported())
    {
        m_entitySetIndex = 2;
        m_materialSetIndex = 3;

        CreatePipelineLayout({ _globalDescriptorSetLayout, _bindlessBindingDescriptorSetLayout, _entityDescriptorSetLayout, m_materialSetLayout->GetDescriptorSetLayout() });
    }
    else
    {
        CreatePipelineLayout({ _globalDescriptorSetLayout, _entityDescriptorSetLayout, m_materialSetLayout->GetDescriptorSetLayout() });
    }
}

PhongRenderSystem::~PhongRenderSystem()
{
    m_app.GetComponentManager().UnregisterComponent<PhongRenderSystem::ColorTintPushConstantData>();
}

void PhongRenderSystem::CreatePipeline(VkRenderPass _renderPass, void(*_config)(PipelineConfigInfo&))
{
    assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    _config(pipelineConfig);

    pipelineConfig.RenderPass = _renderPass;
    pipelineConfig.PipelineLayout = m_pipelineLayout;

    const std::string vertexShaderFilepath = m_device.IsBindlessResourcesSupported()
            ? m_app.GetConfig().ShadersPath + "opaque_shader_bindless1.vert.spv"
            : m_app.GetConfig().ShadersPath + "opaque_shader_bindless0.vert.spv";

    ShaderInfo vertexShader(
            vertexShaderFilepath,
            ShaderType::Vertex);

    const std::string fragmentShaderFilepath = m_device.IsBindlessResourcesSupported()
            ? m_app.GetConfig().ShadersPath + "opaque_shader_bindless1.frag.spv"
            : m_app.GetConfig().ShadersPath + "opaque_shader_bindless0.frag.spv";

    ShaderInfo fragmentShader(
            fragmentShaderFilepath,
            ShaderType::Fragment);

    fragmentShader.AddSpecializationConstant(0, 2.0f);

    m_pipeline = std::make_unique<Pipeline>(
            m_device,
            std::vector{
                    vertexShader,
                    fragmentShader,
            },
            pipelineConfig);
}

void PhongRenderSystem::Cleanup()
{
    for (int32 i = 0; i < m_bindlessBindingMaterialIndexUbos.size(); ++i)
    {
        m_buffer->Destroy(m_bindlessBindingMaterialIndexUbos[i]);
    }
}

VESPERENGINE_NAMESPACE_END
