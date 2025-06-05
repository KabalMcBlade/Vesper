#include "Systems/transparent_render_system.h"

#include "Backend/pipeline.h"
#include "Backend/renderer.h"

VESPERENGINE_NAMESPACE_BEGIN

TransparentRenderSystem::TransparentRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
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

TransparentRenderSystem::~TransparentRenderSystem() = default;

void TransparentRenderSystem::MaterialBinding()
{
    PhongRenderSystem::MaterialBinding<PipelineTransparentComponent>();
}

void TransparentRenderSystem::Update(const FrameInfo& _frameInfo)
{
    PhongRenderSystem::Update<PipelineTransparentComponent>(_frameInfo);
}

void TransparentRenderSystem::Render(const FrameInfo& _frameInfo)
{
    PhongRenderSystem::Render<PipelineTransparentComponent>(_frameInfo);
}

void TransparentRenderSystem::CreatePipeline(VkRenderPass _renderPass)
{
    PhongRenderSystem::CreatePipeline(_renderPass, Pipeline::TransparentPipelineConfiguration);
}

void TransparentRenderSystem::Cleanup()
{
    PhongRenderSystem::Cleanup();
}

VESPERENGINE_NAMESPACE_END
