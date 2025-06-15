// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\phong_transparent_render_system.h
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

class VESPERENGINE_API PhongTransparentRenderSystem : public BaseRenderSystem
{
public:
    static constexpr uint32 kPhongAmbientTextureBindingIndex = 0u;
    static constexpr uint32 kPhongDiffuseTextureBindingIndex = 1u;
    static constexpr uint32 kPhongSpecularTextureBindingIndex = 2u;
    static constexpr uint32 kPhongNormalTextureBindingIndex = 3u;
    static constexpr uint32 kPhongAlphaTextureBindingIndex = 4u;
    static constexpr uint32 kPhongUniformBufferBindingIndex = 5u;

    static constexpr uint32 kPhongUniformBufferOnlyBindingIndex = 0u;

public:
    PhongTransparentRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
            VkDescriptorSetLayout _globalDescriptorSetLayout,
            VkDescriptorSetLayout _entityDescriptorSetLayout,
            VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout = VK_NULL_HANDLE);
    virtual ~PhongTransparentRenderSystem() = default;

    PhongTransparentRenderSystem(const PhongTransparentRenderSystem&) = delete;
    PhongTransparentRenderSystem& operator=(const PhongTransparentRenderSystem&) = delete;

public:
    virtual void CreatePipeline(VkRenderPass _renderPass);
    void MaterialBinding();
    virtual void Update(const FrameInfo& _frameInfo);
    virtual void Render(const FrameInfo& _frameInfo);
    void Cleanup();

protected:
    VesperApp& m_app;
    Renderer& m_renderer;
    std::unique_ptr<Pipeline> m_transparentPipeline;
    std::unique_ptr<DescriptorSetLayout> m_materialSetLayout;

    std::unique_ptr<Buffer> m_buffer;

    std::vector<BufferComponent> m_bindlessBindingMaterialIndexUbos;

    uint32 m_entitySetIndex = 1;
    uint32 m_materialSetIndex = 2;
};

VESPERENGINE_NAMESPACE_END
