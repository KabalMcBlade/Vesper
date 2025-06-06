#pragma once

#include "Core/core_defines.h"
#include "vulkan/vulkan.h"

#include "ECS/ECS/entity.h"

VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API RenderSubsystem
{
public:
    virtual ~RenderSubsystem() = default;

    VESPERENGINE_INLINE virtual VkPushConstantRange GetPushConstantRange() const
    {
        VkPushConstantRange range{};
        return range;
    }

    virtual void Execute(VkCommandBuffer _commandBuffer, VkPipelineLayout _pipelineLayout, ecs::Entity _entity) const = 0;
};

VESPERENGINE_NAMESPACE_END
