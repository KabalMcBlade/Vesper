// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\blend_shape_animation_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/blend_shape_animation_system.h"

#include "Backend/frame_info.h"
#include "Backend/model_data.h"

#include "Components/object_components.h"
#include "App/vesper_app.h"

#include "ECS/ECS/ecs.h"

VESPERENGINE_NAMESPACE_BEGIN

BlendShapeAnimationSystem::BlendShapeAnimationSystem(VesperApp& _app)
    : m_app(_app)
{
}

void BlendShapeAnimationSystem::Update(const FrameInfo& _frameInfo) const
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto entity : ecs::IterateEntitiesWithAll<MorphAnimationComponent, MorphWeightsComponent>(entityManager, componentManager))
    {
        MorphAnimationComponent& animComp = componentManager.GetComponent<MorphAnimationComponent>(entity);
        MorphWeightsComponent& weightsComp = componentManager.GetComponent<MorphWeightsComponent>(entity);

        if (!animComp.Playing || animComp.Animations.empty())
            continue;

        if (animComp.CurrentAnimation >= static_cast<int>(animComp.Animations.size()))
            continue;

        const MorphAnimation& anim = animComp.Animations[animComp.CurrentAnimation];
        if (anim.Keyframes.empty())
            continue;

        animComp.CurrentTime += _frameInfo.FrameTime;

        const MorphKeyframe* k0 = &anim.Keyframes.front();
        const MorphKeyframe* k1 = &anim.Keyframes.back();
        for (size_t i = 1; i < anim.Keyframes.size(); ++i)
        {
            if (animComp.CurrentTime < anim.Keyframes[i].Time)
            {
                k0 = &anim.Keyframes[i - 1];
                k1 = &anim.Keyframes[i];
                break;
            }
        }

        if (animComp.CurrentTime >= anim.Keyframes.back().Time)
        {
            if (animComp.Loop)
            {
                animComp.CurrentTime = 0.0f;
                k0 = &anim.Keyframes.front();
                if (anim.Keyframes.size() > 1)
                    k1 = &anim.Keyframes[1];
            }
            else
            {
                animComp.CurrentTime = anim.Keyframes.back().Time;
                k0 = &anim.Keyframes.back();
                k1 = &anim.Keyframes.back();
            }
        }

        float t = (k1->Time > k0->Time) ? (animComp.CurrentTime - k0->Time) / (k1->Time - k0->Time) : 0.0f;
        weightsComp.Weights[0] = glm::mix(k0->Weights[0], k1->Weights[0], t);
        weightsComp.Weights[1] = glm::mix(k0->Weights[1], k1->Weights[1], t);
    }
}

void BlendShapeAnimationSystem::SetAnimation(ecs::Entity _entity, int _animationIndex) const
{
    if (m_app.GetComponentManager().HasComponents<MorphAnimationComponent>(_entity))
    {
        MorphAnimationComponent& animComp = m_app.GetComponentManager().GetComponent<MorphAnimationComponent>(_entity);
        if (_animationIndex >= 0 && _animationIndex < static_cast<int>(animComp.Animations.size()))
        {
            animComp.CurrentAnimation = _animationIndex;
            animComp.CurrentTime = 0.0f;
        }
    }
}

int32 BlendShapeAnimationSystem::GetAnimationCount(ecs::Entity _entity) const
{
    if (m_app.GetComponentManager().HasComponents<MorphAnimationComponent>(_entity))
    {
        MorphAnimationComponent& animComp = m_app.GetComponentManager().GetComponent<MorphAnimationComponent>(_entity);
        return static_cast<int32>(animComp.Animations.size());
    }

    return -1;
}

void BlendShapeAnimationSystem::SetAnimationForAllEntities(std::vector<int32>& _inOutAnimationIndex) const
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    int32 index = 0;
    for (auto entity : ecs::IterateEntitiesWithAll<MorphAnimationComponent>(entityManager, componentManager))
    {
        MorphAnimationComponent& animComp = componentManager.GetComponent<MorphAnimationComponent>(entity);

        int32 numAnimations = static_cast<int32>(animComp.Animations.size());
        if (numAnimations > 0)
        {
            // Ensure vector is large enough
            if (index >= static_cast<int32>(_inOutAnimationIndex.size()))
            {
                _inOutAnimationIndex.resize(index + 1, 0);
            }

            // Apply current animation index
            int32 animIndex = _inOutAnimationIndex[index] % numAnimations;
            animComp.CurrentAnimation = animIndex;
            animComp.CurrentTime = 0.0f;

            // Increment for next time
            _inOutAnimationIndex[index] = (animIndex + 1) % numAnimations;
        }
        else
        {
            animComp.CurrentAnimation = 0;
            animComp.CurrentTime = 0.0f;
        }

        ++index;
    }
}


VESPERENGINE_NAMESPACE_END