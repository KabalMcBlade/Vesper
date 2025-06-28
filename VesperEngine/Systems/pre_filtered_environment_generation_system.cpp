// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\pre_filtered_environment_generation_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/pre_filtered_environment_generation_system.h"

#include "Backend/device.h"
#include "Backend/model_data.h"
#include "Backend/buffer.h"
#include "Backend/pipeline.h"
#include "Backend/descriptors.h"

#include "App/vesper_app.h"
#include "App/config.h"

#include <array>

VESPERENGINE_NAMESPACE_BEGIN

namespace
{
    struct PushConstant
    {
        glm::mat4 ViewProjection{ 1.f };
        float Roughness{ 0.f };
        uint32 NumSamples{ 32u };
    };

    static std::vector<Vertex> CreateCubeVertices()
    {
        const std::array<glm::vec3, 36> positions{
            glm::vec3{-0.5f,-0.5f, 0.5f}, glm::vec3{ 0.5f, 0.5f, 0.5f}, glm::vec3{-0.5f, 0.5f, 0.5f},
            glm::vec3{-0.5f,-0.5f, 0.5f}, glm::vec3{ 0.5f,-0.5f, 0.5f}, glm::vec3{ 0.5f, 0.5f, 0.5f},
            glm::vec3{-0.5f,-0.5f,-0.5f}, glm::vec3{-0.5f, 0.5f,-0.5f}, glm::vec3{ 0.5f, 0.5f,-0.5f},
            glm::vec3{-0.5f,-0.5f,-0.5f}, glm::vec3{ 0.5f, 0.5f,-0.5f}, glm::vec3{ 0.5f,-0.5f,-0.5f},
            glm::vec3{-0.5f,-0.5f,-0.5f}, glm::vec3{-0.5f, 0.5f, 0.5f}, glm::vec3{-0.5f, 0.5f,-0.5f},
            glm::vec3{-0.5f,-0.5f,-0.5f}, glm::vec3{-0.5f,-0.5f, 0.5f}, glm::vec3{-0.5f, 0.5f, 0.5f},
            glm::vec3{ 0.5f,-0.5f,-0.5f}, glm::vec3{ 0.5f, 0.5f,-0.5f}, glm::vec3{ 0.5f, 0.5f, 0.5f},
            glm::vec3{ 0.5f,-0.5f,-0.5f}, glm::vec3{ 0.5f, 0.5f, 0.5f}, glm::vec3{ 0.5f,-0.5f, 0.5f},
            glm::vec3{-0.5f, 0.5f,-0.5f}, glm::vec3{-0.5f, 0.5f, 0.5f}, glm::vec3{ 0.5f, 0.5f, 0.5f},
            glm::vec3{-0.5f, 0.5f,-0.5f}, glm::vec3{ 0.5f, 0.5f, 0.5f}, glm::vec3{ 0.5f, 0.5f,-0.5f},
            glm::vec3{-0.5f,-0.5f,-0.5f}, glm::vec3{ 0.5f,-0.5f, 0.5f}, glm::vec3{-0.5f,-0.5f, 0.5f},
            glm::vec3{-0.5f,-0.5f,-0.5f}, glm::vec3{ 0.5f,-0.5f,-0.5f}, glm::vec3{ 0.5f,-0.5f, 0.5f}
        };
        std::vector<Vertex> verts(36);
        for (uint32 i = 0; i < 36; ++i) verts[i].Position = positions[i];
        return verts;
    }
}

PreFilteredEnvironmentGenerationSystem::PreFilteredEnvironmentGenerationSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass)
    : BaseRenderSystem(_device)
    , m_app(_app)
{
    m_buffer = std::make_unique<Buffer>(m_device);
    CreateDescriptorResources();

    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    range.offset = 0;
    range.size = sizeof(PushConstant);
    m_pushConstants.push_back(range);

    CreatePipelineLayout({ m_setLayout->GetDescriptorSetLayout() });
    CreatePipeline(_renderPass);
}

PreFilteredEnvironmentGenerationSystem::~PreFilteredEnvironmentGenerationSystem()
{
    if (m_cubeVertex.Buffer)
        m_buffer->Destroy(m_cubeVertex);
}

void PreFilteredEnvironmentGenerationSystem::CreateDescriptorResources()
{
    m_setLayout = DescriptorSetLayout::Builder(m_device)
        .AddBinding(kEnvironmentMapBindingIndex, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();
    m_descriptorPool = DescriptorPool::Builder(m_device)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        .SetMaxSets(1)
        .Build();
}

void PreFilteredEnvironmentGenerationSystem::CreatePipeline(VkRenderPass _renderPass)
{
    PipelineConfigInfo config{};
    Pipeline::SkyboxPipelineConfig(config);
    config.RenderPass = _renderPass;
    config.PipelineLayout = m_pipelineLayout;
    config.DepthStencilInfo.depthTestEnable = VK_FALSE;
    config.DepthStencilInfo.depthWriteEnable = VK_FALSE;
    config.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;

    ShaderInfo vert(m_app.GetConfig().ShadersPath + "cubemap_shader.vert.spv", ShaderType::Vertex);
    ShaderInfo frag(m_app.GetConfig().ShadersPath + "pre_filtered_environment_map.frag.spv", ShaderType::Fragment);
    m_pipeline = std::make_unique<Pipeline>(m_device, std::vector{ vert, frag }, config);
}

void PreFilteredEnvironmentGenerationSystem::Generate(
    VkCommandBuffer _commandBuffer,
    const VkDescriptorImageInfo& _envMapInfo,
    const glm::mat4& _viewProj,
    float _roughness,
    uint32 _numSamples) 
{
    if (m_cubeVertex.Buffer == VK_NULL_HANDLE)
    {
        std::vector<Vertex> vertices = CreateCubeVertices();
        const uint32 vertexCount = static_cast<uint32>(vertices.size());
        const VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;
        const uint32 vertexSize = sizeof(Vertex);
        m_cubeVertex = m_buffer->Create<VertexBufferComponent>(vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
        m_buffer->Map(m_cubeVertex);
        m_buffer->WriteToBuffer(m_cubeVertex.MappedMemory, vertices.data(), bufferSize);
        m_buffer->Unmap(m_cubeVertex);
    }

    if (m_descriptorSet == VK_NULL_HANDLE)
    {
        DescriptorWriter(*m_setLayout, *m_descriptorPool)
            .WriteImage(kEnvironmentMapBindingIndex, const_cast<VkDescriptorImageInfo*>(&_envMapInfo))
            .Build(m_descriptorSet);
    }

    m_pipeline->Bind(_commandBuffer);
    vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);

    PushConstant pc{ _viewProj, _roughness, _numSamples };
    PushConstants(_commandBuffer, 0, &pc);

    //if (vkCmdSetCullModeEXT && vkCmdSetFrontFaceEXT)  // no need, we do throw and exception if not supported
    {
        vkCmdSetCullModeEXT(_commandBuffer, VK_CULL_MODE_NONE);
        vkCmdSetFrontFaceEXT(_commandBuffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    }

    Bind(m_cubeVertex, _commandBuffer);
    Draw(m_cubeVertex, _commandBuffer);
}

VESPERENGINE_NAMESPACE_END
