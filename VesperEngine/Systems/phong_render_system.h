#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"

#include "Systems/base_render_system.h"

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

class VESPERENGINE_API PhongRenderSystem : public BaseRenderSystem
{
public:
    static constexpr uint32 kPhongAmbientTextureBindingIndex = 0u;
    static constexpr uint32 kPhongDiffuseTextureBindingIndex = 1u;
    static constexpr uint32 kPhongSpecularTextureBindingIndex = 2u;
    static constexpr uint32 kPhongNormalTextureBindingIndex = 3u;
    static constexpr uint32 kPhongUniformBufferBindingIndex = 4u;

    static constexpr uint32 kPhongUniformBufferOnlyBindingIndex = 0u;

    struct ColorTintPushConstantData
    {
        glm::vec3 ColorTint{1.0f};
    };

    PhongRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
            VkDescriptorSetLayout _globalDescriptorSetLayout,
            VkDescriptorSetLayout _entityDescriptorSetLayout,
            VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout = VK_NULL_HANDLE);
    virtual ~PhongRenderSystem();

    PhongRenderSystem(const PhongRenderSystem&) = delete;
    PhongRenderSystem& operator=(const PhongRenderSystem&) = delete;

protected:
    template <typename PipelineTagComponent>
    void MaterialBinding();

    template <typename PipelineTagComponent>
    void Update(const FrameInfo& _frameInfo);

    template <typename PipelineTagComponent>
    void Render(const FrameInfo& _frameInfo);

    void Cleanup();

    void CreatePipeline(VkRenderPass _renderPass, void(*_config)(PipelineConfigInfo&));

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

#include "phong_render_system.inl"
