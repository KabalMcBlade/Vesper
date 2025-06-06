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
class RenderSubsystem;
class DefaultColorTintSubsystem;

struct FrameInfo;
struct BufferComponent;

class VESPERENGINE_API TransparentRenderSystem final : public BaseRenderSystem
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
    TransparentRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
            VkDescriptorSetLayout _globalDescriptorSetLayout,
            VkDescriptorSetLayout _entityDescriptorSetLayout,
            VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout = VK_NULL_HANDLE,
            const std::vector<RenderSubsystem*>& _subsystems = {});
    ~TransparentRenderSystem() = default;

    TransparentRenderSystem(const TransparentRenderSystem&) = delete;
    TransparentRenderSystem& operator=(const TransparentRenderSystem&) = delete;

public:
    void MaterialBinding();
    void Update(const FrameInfo& _frameInfo);
    void Render(const FrameInfo& _frameInfo);
    void Cleanup();

private:
    void CreatePipeline(VkRenderPass _renderPass);

private:
    VesperApp& m_app;
    Renderer& m_renderer;
    std::unique_ptr<Pipeline> m_transparentPipeline;
    std::unique_ptr<DescriptorSetLayout> m_materialSetLayout;

    std::unique_ptr<Buffer> m_buffer;

    std::vector<BufferComponent> m_bindlessBindingMaterialIndexUbos;

    uint32 m_entitySetIndex = 1;
    uint32 m_materialSetIndex = 2;

    std::unique_ptr<DefaultColorTintSubsystem> m_defaultColorTintSubsystem;

};

VESPERENGINE_NAMESPACE_END
