#include "Systems/skybox_render_system.h"
#include "Systems/camera_system.h"
#include "Backend/pipeline.h"
#include "Backend/renderer.h"
#include "Backend/buffer.h"
#include "Backend/descriptors.h"
#include "Backend/model_data.h"
#include "Systems/texture_system.h"
#include "Backend/device.h"
#include "Backend/frame_info.h"
#include "Components/camera_components.h"
#include "App/vesper_app.h"
#include "App/config.h"

SkyboxRenderSystem::SkyboxRenderSystem(VesperApp& _app,
                                       Device& _device,
                                       Renderer& _renderer,
                                       VkDescriptorSetLayout _globalDescriptorSetLayout,
                                       VkDescriptorSetLayout _bindlessDescriptorSetLayout)

    std::vector<VkDescriptorSetLayout> setLayouts{ _globalDescriptorSetLayout };
    if (m_device.IsBindlessResourcesSupported())
    {
        setLayouts.push_back(_bindlessDescriptorSetLayout);
    }
    setLayouts.push_back(m_textureSetLayout->GetDescriptorSetLayout());
    CreatePipelineLayout(setLayouts);

    uint32_t textureSetIndex = m_device.IsBindlessResourcesSupported() ? 2 : 1;
        textureSetIndex,
        Vertex{{-10.f,-10.f,-10.f}}, Vertex{{10.f,-10.f,10.f}}, Vertex{{-10.f,-10.f,10.f}},
        Vertex{{-10.f,-10.f,-10.f}}, Vertex{{10.f,-10.f,-10.f}}, Vertex{{10.f,-10.f,10.f}},
        // bottom
        Vertex{{-10.f,10.f,-10.f}}, Vertex{{10.f,10.f,10.f}}, Vertex{{-10.f,10.f,10.f}},
        Vertex{{-10.f,10.f,-10.f}}, Vertex{{10.f,10.f,-10.f}}, Vertex{{10.f,10.f,10.f}},
        // nose
        Vertex{{-10.f,-10.f,10.f}}, Vertex{{10.f,10.f,10.f}}, Vertex{{-10.f,10.f,10.f}},
        Vertex{{-10.f,-10.f,10.f}}, Vertex{{10.f,-10.f,10.f}}, Vertex{{10.f,10.f,10.f}},
        // tail
        Vertex{{-10.f,-10.f,-10.f}}, Vertex{{10.f,10.f,-10.f}}, Vertex{{-10.f,10.f,-10.f}},
        Vertex{{-10.f,-10.f,-10.f}}, Vertex{{10.f,-10.f,-10.f}}, Vertex{{10.f,10.f,-10.f}},
    };
}

SkyboxRenderSystem::SkyboxRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer, VkDescriptorSetLayout _globalDescriptorSetLayout)
    : BaseRenderSystem(_device)
    , m_app(_app)
    , m_renderer(_renderer)
{
    m_buffer = std::make_unique<Buffer>(m_device);

    m_descriptorPool = DescriptorPool::Builder(m_device)
        .SetMaxSets(1)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        .Build();

    m_textureSetLayout = DescriptorSetLayout::Builder(m_device)
        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();

    VkPushConstantRange push{};
    push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push.offset = 0;
    push.size = sizeof(glm::mat4);
    m_pushConstants.push_back(push);

    CreatePipelineLayout({ _globalDescriptorSetLayout, m_textureSetLayout->GetDescriptorSetLayout() });
    CreatePipeline(m_renderer.GetSwapChainRenderPass());

    const VkDeviceSize bufferSize = sizeof(kVertices[0]) * kVertices.size();
    const uint32 vertexSize = sizeof(kVertices[0]);
    m_vertexBuffer = m_buffer->Create<VertexBufferComponent>(
        vertexSize,
        static_cast<uint32>(kVertices.size()),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
    m_buffer->Map(m_vertexBuffer);
    m_buffer->WriteToBuffer(m_vertexBuffer.MappedMemory, (void*)kVertices.data(), bufferSize);
    m_buffer->Unmap(m_vertexBuffer);
}

SkyboxRenderSystem::~SkyboxRenderSystem()
{
}

void SkyboxRenderSystem::SetCubemapTexture(std::shared_ptr<TextureData> _cubemap)
{
    m_cubemap = std::move(_cubemap);
    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = m_cubemap->Sampler;
    imageInfo.imageView = m_cubemap->ImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    DescriptorWriter(*m_textureSetLayout, *m_descriptorPool)
        .WriteImage(0, &imageInfo)
        .Build(m_textureDescriptorSet);
}

void SkyboxRenderSystem::Render(const FrameInfo& _frameInfo, const CameraComponent& _camera)
{
    if (!m_cubemap)
        return;

    m_pipeline->Bind(_frameInfo.CommandBuffer);

    vkCmdBindDescriptorSets(
        _frameInfo.CommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout,
        0,
        1,
        &_frameInfo.GlobalDescriptorSet,
        0,
        nullptr);

    vkCmdBindDescriptorSets(
        _frameInfo.CommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout,
        1,
        1,
        &m_textureDescriptorSet,
        0,
        nullptr);

    glm::mat4 view = glm::mat4(glm::mat3(_camera.ViewMatrix));
    glm::mat4 vp = _camera.ProjectionMatrix * view;
    PushConstants(_frameInfo.CommandBuffer, 0, &vp);

    Bind(m_vertexBuffer, _frameInfo.CommandBuffer);
    Draw(m_vertexBuffer, _frameInfo.CommandBuffer);
}

void SkyboxRenderSystem::Cleanup()
{
    m_buffer->Destroy(m_vertexBuffer);
}

void SkyboxRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
    PipelineConfigInfo config{};
    Pipeline::SkyboxPipelineConfig(config);
    config.RenderPass = _renderPass;
    config.PipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(
        m_device,
        std::vector{ ShaderInfo{m_app.GetConfig().ShadersPath + "cubemap_shader.vert.spv", ShaderType::Vertex},
                     ShaderInfo{m_app.GetConfig().ShadersPath + "skybox_shader.frag.spv", ShaderType::Fragment} },
        config);
}

VESPERENGINE_NAMESPACE_END
