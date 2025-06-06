#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"
#include "vulkan/vulkan.h"

#include "ECS/ECS/entity.h"
#include "Systems/render_subsystem.h"


VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
struct FrameInfo;

struct ColorTintPushConstantData
{
    glm::vec3 ColorTint{1.0f};
};

class VESPERENGINE_API ColorTintSystem final : public RenderSubsystem
{
public:
    explicit ColorTintSystem(VesperApp& _app);
    ~ColorTintSystem();

    ColorTintSystem(const ColorTintSystem&) = delete;
    ColorTintSystem& operator=(const ColorTintSystem&) = delete;

    void AddColorComponents();
    void Update(const FrameInfo& _frameInfo);
    void Execute(VkCommandBuffer _commandBuffer, VkPipelineLayout _pipelineLayout, ecs::Entity _entity) const override;
    VESPERENGINE_INLINE VkPushConstantRange GetPushConstantRange() const override;

private:
    VesperApp& m_app;
};

VESPERENGINE_INLINE VkPushConstantRange ColorTintSystem::GetPushConstantRange() const
{
    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    range.offset = 0;
    range.size = sizeof(ColorTintPushConstantData);
    return range;
}

VESPERENGINE_NAMESPACE_END
