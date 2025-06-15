// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\pbr_opaque_render_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Systems/base_render_system.h"
#include "vulkan/vulkan.h"

#include <memory>
#include <vector>

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;
class Renderer;
class Pipeline;
class DescriptorSetLayout;
class Buffer;

struct FrameInfo;
struct BufferComponent;

class VESPERENGINE_API PBROpaqueRenderSystem : public BaseRenderSystem
{
public:
    static constexpr uint32 kPBRRoughnessTextureBindingIndex = 0u;
    static constexpr uint32 kPBRMetallicTextureBindingIndex = 1u;
    static constexpr uint32 kPBRSheenTextureBindingIndex = 2u;
    static constexpr uint32 kPBREmissiveTextureBindingIndex = 3u;
    static constexpr uint32 kPBRNormalTextureBindingIndex = 4u;
    static constexpr uint32 kPBRUniformBufferBindingIndex = 5u;

    static constexpr uint32 kPBRUniformBufferOnlyBindingIndex = 0u;

public:
    PBROpaqueRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
        VkDescriptorSetLayout _globalDescriptorSetLayout,
        VkDescriptorSetLayout _entityDescriptorSetLayout,
        VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout = VK_NULL_HANDLE);
    virtual ~PBROpaqueRenderSystem() = default;

    PBROpaqueRenderSystem(const PBROpaqueRenderSystem&) = delete;
    PBROpaqueRenderSystem& operator=(const PBROpaqueRenderSystem&) = delete;

public:
    virtual void CreatePipeline(VkRenderPass _renderPass);
    void MaterialBinding();
    virtual void Update(const FrameInfo& _frameInfo);
    virtual void Render(const FrameInfo& _frameInfo);
    void Cleanup();

protected:
    VesperApp& m_app;
    Renderer& m_renderer;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<DescriptorSetLayout> m_materialSetLayout;

    std::unique_ptr<Buffer> m_buffer;

    std::vector<BufferComponent> m_bindlessBindingMaterialIndexUbos;

    uint32 m_entitySetIndex = 1;
    uint32 m_materialSetIndex = 2;
};

VESPERENGINE_NAMESPACE_END
