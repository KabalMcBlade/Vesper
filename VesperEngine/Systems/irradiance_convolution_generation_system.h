// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\irradiance_convolution_generation_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"
#include "Systems/base_render_system.h"
#include "Components/graphics_components.h"

#include <memory>

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;
class Buffer;
class Pipeline;
class DescriptorSetLayout;
class DescriptorPool;

class VESPERENGINE_API IrradianceConvolutionGenerationSystem : public BaseRenderSystem
{
public:
    static constexpr uint32 kEnvironmentMapBindingIndex = 0u;

    IrradianceConvolutionGenerationSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass);
    ~IrradianceConvolutionGenerationSystem();

    IrradianceConvolutionGenerationSystem(const IrradianceConvolutionGenerationSystem&) = delete;
    IrradianceConvolutionGenerationSystem& operator=(const IrradianceConvolutionGenerationSystem&) = delete;

    void Generate(VkCommandBuffer _commandBuffer, const VkDescriptorImageInfo& _envMapInfo, const glm::mat4& _viewProj);

private:
    void CreateDescriptorResources();
    void CreatePipeline(VkRenderPass _renderPass);

private:
    VesperApp& m_app;
    std::unique_ptr<Buffer> m_buffer;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<DescriptorSetLayout> m_setLayout;
    std::unique_ptr<DescriptorPool> m_descriptorPool;
    VkDescriptorSet m_descriptorSet{ VK_NULL_HANDLE };
    VertexBufferComponent m_cubeVertex{};
};

VESPERENGINE_NAMESPACE_END
