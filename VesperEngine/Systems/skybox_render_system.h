#pragma once

#include "Core/core_defines.h"
#include "Components/graphics_components.h"
#include "Systems/base_render_system.h"

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;
class Renderer;
class DescriptorSetLayout;
class DescriptorPool;
class Pipeline;
class Buffer;
struct TextureData;
struct CameraComponent;
struct FrameInfo;
struct VertexBufferComponent;

class VESPERENGINE_API SkyboxRenderSystem final : public BaseRenderSystem
{
public:
    SkyboxRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer, VkDescriptorSetLayout _globalDescriptorSetLayout);
    ~SkyboxRenderSystem();

    SkyboxRenderSystem(const SkyboxRenderSystem&) = delete;
    SkyboxRenderSystem& operator=(const SkyboxRenderSystem&) = delete;

public:
    void SetCubemapTexture(std::shared_ptr<TextureData> _cubemap);
    void Render(const FrameInfo& _frameInfo, const CameraComponent& _camera);
    void Cleanup();

private:
    void CreatePipeline(VkRenderPass _renderPass);

private:
    VesperApp& m_app;
    Renderer& m_renderer;

    std::unique_ptr<DescriptorPool> m_descriptorPool;
    std::unique_ptr<DescriptorSetLayout> m_textureSetLayout;
    VkDescriptorSet m_textureDescriptorSet{ VK_NULL_HANDLE };

    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<Buffer> m_buffer;
    VertexBufferComponent m_vertexBuffer{};
    std::shared_ptr<TextureData> m_cubemap;
};

VESPERENGINE_NAMESPACE_END
