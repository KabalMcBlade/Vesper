#pragma once

#include "Core/core_defines.h"
#include "Systems/base_render_system.h"

#include <memory>
#include <vector>

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;
class Renderer;
class Pipeline;
class DescriptorSetLayout;

struct FrameInfo;

class VESPERENGINE_API SkyboxRenderSystem : public BaseRenderSystem
{
public:
    static constexpr uint32 kCubemapBindingIndex = 0u;

    SkyboxRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
        VkDescriptorSetLayout _globalDescriptorSetLayout,
        VkDescriptorSetLayout _bindlessDescriptorSetLayout = VK_NULL_HANDLE);
    ~SkyboxRenderSystem() = default;

    SkyboxRenderSystem(const SkyboxRenderSystem&) = delete;
    SkyboxRenderSystem& operator=(const SkyboxRenderSystem&) = delete;

    void CreatePipeline(VkRenderPass _renderPass);
    void MaterialBinding();
    void Update(const FrameInfo& _frameInfo);
    void Render(const FrameInfo& _frameInfo);
    void Cleanup();

private:
    VesperApp& m_app;
    Renderer& m_renderer;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<DescriptorSetLayout> m_setLayout;
    uint32 m_skyboxSetIndex = 1;
};

VESPERENGINE_NAMESPACE_END
