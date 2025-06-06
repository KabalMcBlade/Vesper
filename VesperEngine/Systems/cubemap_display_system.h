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
struct CameraComponent;
struct TextureData;
struct VertexBufferComponent;

class VESPERENGINE_API CubemapDisplaySystem final : public BaseRenderSystem
{
public:
    static constexpr uint32 kCubemapBindingIndex = 0u;

    CubemapDisplaySystem(VesperApp& app, Device& device, Renderer& renderer,
                         VkDescriptorSetLayout globalDescriptorSetLayout,
                         VkDescriptorSetLayout bindlessDescriptorSetLayout = VK_NULL_HANDLE);
    ~CubemapDisplaySystem();

    CubemapDisplaySystem(const CubemapDisplaySystem&) = delete;
    CubemapDisplaySystem& operator=(const CubemapDisplaySystem&) = delete;

    void AddCubemap(std::shared_ptr<TextureData> cubemap, glm::vec3 offset, float scale);

    void Update(const CameraComponent& cameraComponent);
    void Render(const FrameInfo& frameInfo);
    void Cleanup();

private:
    struct CubemapObject
    {
        std::shared_ptr<TextureData> Cubemap;
        VertexBufferComponent VertexBuffer;
        std::vector<VkDescriptorSet> DescriptorSets;
    };

    void CreatePipeline(VkRenderPass renderPass);
    VertexBufferComponent CreateCubeBuffer(glm::vec3 offset, float scale) const;

private:
    VesperApp& m_app;
    Renderer& m_renderer;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<DescriptorSetLayout> m_cubemapSetLayout;
    std::unique_ptr<Buffer> m_buffer;

    uint32 m_cubemapSetIndex = 1;
    std::vector<CubemapObject> m_cubemapObjects;
};

VESPERENGINE_NAMESPACE_END
