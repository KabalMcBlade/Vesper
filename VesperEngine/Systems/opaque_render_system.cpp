#include "Systems/opaque_render_system.h"

#include "Backend/pipeline.h"
#include "Backend/renderer.h"

VESPERENGINE_NAMESPACE_BEGIN

OpaqueRenderSystem::OpaqueRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
        VkDescriptorSetLayout _globalDescriptorSetLayout,
        VkDescriptorSetLayout _entityDescriptorSetLayout,
        VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout)
    : PhongRenderSystem(_app, _device, _renderer,
            _globalDescriptorSetLayout,
            _entityDescriptorSetLayout,
            _bindlessBindingDescriptorSetLayout)
{
    CreatePipeline(_renderer.GetSwapChainRenderPass());
}

OpaqueRenderSystem::~OpaqueRenderSystem() = default;

void OpaqueRenderSystem::MaterialBinding()
{
    PhongRenderSystem::MaterialBinding<PipelineOpaqueComponent>();
}

void OpaqueRenderSystem::Update(const FrameInfo& _frameInfo)
{
    PhongRenderSystem::Update<PipelineOpaqueComponent>(_frameInfo);
}

void OpaqueRenderSystem::Render(const FrameInfo& _frameInfo)
{
    PhongRenderSystem::Render<PipelineOpaqueComponent>(_frameInfo);
}

void OpaqueRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
    PhongRenderSystem::CreatePipeline(_renderPass, Pipeline::OpaquePipelineConfiguration);
}

void OpaqueRenderSystem::Cleanup()
{
    PhongRenderSystem::Cleanup();
}

VESPERENGINE_NAMESPACE_END
