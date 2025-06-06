#include "Systems/color_tint_system.h"

#include "Backend/frame_info.h"

#include "Components/object_components.h"
#include "Components/pipeline_components.h"

#include "App/vesper_app.h"

#include "ECS/ECS/ecs.h"

#include <cmath>

VESPERENGINE_NAMESPACE_BEGIN

ColorTintSystem::ColorTintSystem(VesperApp& _app)
    : m_app(_app)
{
    m_app.GetComponentManager().RegisterComponent<ColorTintPushConstantData>();
}

ColorTintSystem::~ColorTintSystem()
{
    m_app.GetComponentManager().UnregisterComponent<ColorTintPushConstantData>();
}

void ColorTintSystem::AddColorComponents()
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto gameEntity : ecs::IterateEntitiesWithAny<PipelineOpaqueComponent, PipelineTransparentComponent>(entityManager, componentManager))
    {
        if (!componentManager.HasComponents<ColorTintPushConstantData>(gameEntity))
        {
            componentManager.AddComponent<ColorTintPushConstantData>(gameEntity);
        }
    }
}

void ColorTintSystem::Update(const FrameInfo& _frameInfo)
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto gameEntity : ecs::IterateEntitiesWithAll<ColorTintPushConstantData>(entityManager, componentManager))
    {
        ColorTintPushConstantData& pushComponent = componentManager.GetComponent<ColorTintPushConstantData>(gameEntity);

        static const float speed = 1.0f;
        static float frameTimeUpdated = 0.0f;
        const float r = 0.5f * (std::sin(speed * frameTimeUpdated) + 1.0f);
        const float g = 0.5f * (std::sin(speed * frameTimeUpdated + glm::pi<float>() / 3.0f) + 1.0f);
        const float b = 0.5f * (std::sin(speed * frameTimeUpdated + 2.0f * glm::pi<float>() / 3.0f) + 1.0f);
        frameTimeUpdated += _frameInfo.FrameTime;
        pushComponent.ColorTint = glm::vec3(r, g, b);
    }
}

void ColorTintSystem::Execute(VkCommandBuffer _commandBuffer, VkPipelineLayout _pipelineLayout, ecs::Entity _entity) const
{
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    ColorTintPushConstantData pushComponent{};
    if (componentManager.HasComponents<ColorTintPushConstantData>(_entity))
    {
        pushComponent = componentManager.GetComponent<ColorTintPushConstantData>(_entity);
    }

    vkCmdPushConstants(
            _commandBuffer,
            _pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(ColorTintPushConstantData),
            &pushComponent);
}

void DefaultColorTintSubsystem::Execute(VkCommandBuffer _commandBuffer, VkPipelineLayout _pipelineLayout, ecs::Entity) const
{
    ColorTintPushConstantData defaultData{};
    vkCmdPushConstants(
            _commandBuffer,
            _pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(ColorTintPushConstantData),
            &defaultData);
}

VESPERENGINE_NAMESPACE_END
