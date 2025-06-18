#ifndef VESPERENGINE_TRANSFORM_H
#define VESPERENGINE_TRANSFORM_H

#include "Core/core_defines.h"
#include "Core/glm_config.h"
#include "ECS/ECS/ecs.h"
#include "Components/object_components.h"

VESPERENGINE_NAMESPACE_BEGIN

inline glm::mat4 GetLocalMatrix(const TransformComponent& _transform)
{
    glm::mat4 model = glm::translate(glm::mat4{1.0f}, _transform.Position);
    model = model * glm::toMat4(_transform.Rotation);
    model = glm::scale(model, _transform.Scale);
    return model;
}

inline glm::mat4 GetWorldMatrix(const ecs::Entity _entity, ecs::ComponentManager& _componentManager)
{
    const TransformComponent& transform = _componentManager.GetComponent<TransformComponent>(_entity);
    glm::mat4 model = GetLocalMatrix(transform);
    if (transform.Parent != ecs::UnknowEntity &&
        _componentManager.HasComponents<TransformComponent>(transform.Parent))
    {
        model = GetWorldMatrix(transform.Parent, _componentManager) * model;
    }
    return model;
}

VESPERENGINE_NAMESPACE_END

#endif // VESPERENGINE_TRANSFORM_H
