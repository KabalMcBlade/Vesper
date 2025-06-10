#include "Systems/cubemap_display_system.h"

#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/pipeline.h"
#include "Backend/renderer.h"
#include "Backend/descriptors.h"
#include "Backend/model_data.h"
#include "Backend/frame_info.h"

#include "App/vesper_app.h"
#include "App/config.h"

#include "Utility/primitive_factory.h"

#include "Components/graphics_components.h"
#include "Components/camera_components.h"


VESPERENGINE_NAMESPACE_BEGIN

CubemapDisplaySystem::CubemapDisplaySystem(VesperApp& app, Device& device, Renderer& renderer,
                                           MaterialSystem& materialSystem,
                                           VkDescriptorSetLayout globalDescriptorSetLayout,
                                           VkDescriptorSetLayout bindlessDescriptorSetLayout)
    : BaseRenderSystem(device), m_app(app), m_renderer(renderer), m_materialSystem(materialSystem)
{
    // TEMPORARY, WILL BE REMOVED!
    m_defaultColorTintSubsystem = std::make_unique<DefaultColorTintSubsystem>();
    AddRenderSubsystem(m_defaultColorTintSubsystem.get());

    m_buffer = std::make_unique<Buffer>(m_device);

    m_cubemapSetLayout = DescriptorSetLayout::Builder(m_device)
            .AddBinding(kCubemapBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();

    if (m_device.IsBindlessResourcesSupported() && bindlessDescriptorSetLayout != VK_NULL_HANDLE)
    {
        m_cubemapSetIndex = 2;
        CreatePipelineLayout({ globalDescriptorSetLayout, bindlessDescriptorSetLayout,
                               m_cubemapSetLayout->GetDescriptorSetLayout() });
    }
    else
    {
        m_cubemapSetIndex = 1;
        CreatePipelineLayout({ globalDescriptorSetLayout, m_cubemapSetLayout->GetDescriptorSetLayout() });
    }

    CreatePipeline(m_renderer.GetSwapChainRenderPass());
}

CubemapDisplaySystem::~CubemapDisplaySystem()
{
}

void CubemapDisplaySystem::AddCubemap(std::shared_ptr<TextureData> cubemap, glm::vec3 offset, float scale)
{
    CubemapObject obj{};
    obj.Cubemap = cubemap;
    obj.VertexBuffer = CreateCubeBuffer(offset, scale);
    obj.DescriptorSets.resize(SwapChain::kMaxFramesInFlight);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = cubemap->Sampler;
    imageInfo.imageView = cubemap->ImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    for(int i=0;i<SwapChain::kMaxFramesInFlight;++i)
    {
        DescriptorWriter(*m_cubemapSetLayout, *m_renderer.GetDescriptorPool())
            .WriteImage(kCubemapBindingIndex, &imageInfo)
            .Build(obj.DescriptorSets[i]);
    }

    m_cubemapObjects.push_back(std::move(obj));
}

void CubemapDisplaySystem::Update(const CameraComponent& /*cameraComponent*/)
{
}

void CubemapDisplaySystem::Render(const FrameInfo& frameInfo)
{
    m_pipeline->Bind(frameInfo.CommandBuffer);

    for (const auto& obj : m_cubemapObjects)
    {
        vkCmdBindDescriptorSets(frameInfo.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipelineLayout, m_cubemapSetIndex, 1,
                                &obj.DescriptorSets[frameInfo.FrameIndex], 0, nullptr);

        Bind(obj.VertexBuffer, frameInfo.CommandBuffer);
        Draw(obj.VertexBuffer, frameInfo.CommandBuffer);
    }
}

void CubemapDisplaySystem::Cleanup()
{
    for(auto& obj : m_cubemapObjects)
    {
        m_buffer->Destroy(obj.VertexBuffer);
    }
    m_cubemapObjects.clear();
}

VertexBufferComponent CubemapDisplaySystem::CreateCubeBuffer(glm::vec3 offset, float scale) const
{
    std::unique_ptr<ModelData> model = PrimitiveFactory::GenerateCubeNoIndices(
            m_materialSystem, glm::vec3{0.0f}, glm::vec3{1.0f});
    auto verts = std::move(model->Vertices);
    for (auto& v : verts)
    {
        v.Position = v.Position * scale + offset;
    }
    const uint32 vertexCount = static_cast<uint32>(verts.size());
    const VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;
    const uint32 vertexSize = sizeof(Vertex);

    BufferComponent staging = m_buffer->Create<BufferComponent>(
            vertexSize, vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);

    m_buffer->Map(staging);
    m_buffer->WriteToBuffer(staging.MappedMemory, verts.data(), bufferSize);
    m_buffer->Unmap(staging);

    VertexBufferComponent vb = m_buffer->Create<VertexBufferComponent>(
            vertexSize, vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);

    m_buffer->Copy(staging, vb, bufferSize);
    m_buffer->Destroy(staging);
    return vb;
}

void CubemapDisplaySystem::CreatePipeline(VkRenderPass renderPass)
{
    PipelineConfigInfo config{};
    Pipeline::SkyboxPipelineConfig(config);
    config.RenderPass = renderPass;
    config.PipelineLayout = m_pipelineLayout;

    const std::string vertexShaderFilepath = m_device.IsBindlessResourcesSupported()
        ? m_app.GetConfig().ShadersPath + "cubemap_shader_bindless1.vert.spv"
        : m_app.GetConfig().ShadersPath + "cubemap_shader_bindless0.vert.spv";

    ShaderInfo vertexShader(
        vertexShaderFilepath,
        ShaderType::Vertex
    );

    const std::string fragmentShaderFilepath = m_device.IsBindlessResourcesSupported()
        ? m_app.GetConfig().ShadersPath + "cubemap_shader_bindless1.frag.spv"
        : m_app.GetConfig().ShadersPath + "cubemap_shader_bindless0.frag.spv";

    ShaderInfo fragmentShader(
        fragmentShaderFilepath,
        ShaderType::Fragment
    );

    m_pipeline = std::make_unique<Pipeline>(
            m_device,
            std::vector{
                vertexShader,
                fragmentShader
            },
            config);
}

VESPERENGINE_NAMESPACE_END
