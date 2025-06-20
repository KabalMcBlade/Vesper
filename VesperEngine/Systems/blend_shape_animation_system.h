// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\blend_shape_animation_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "ECS/ECS/entity.h"

#include <vector>

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;

struct FrameInfo;

class VESPERENGINE_API BlendShapeAnimationSystem final
{
public:
    BlendShapeAnimationSystem(VesperApp& _app);
    ~BlendShapeAnimationSystem() = default;

    BlendShapeAnimationSystem(const BlendShapeAnimationSystem&) = delete;
    BlendShapeAnimationSystem& operator=(const BlendShapeAnimationSystem&) = delete;

    void Update(const FrameInfo& _frameInfo) const;
    void SetAnimation(ecs::Entity _entity, int32 _animationIndex) const;

    int32 GetAnimationCount(ecs::Entity _entity) const;


    // Debug
public:
    void SetAnimationForAllEntities(std::vector<int32>& _inOutAnimationIndex) const;

private:
    VesperApp& m_app;
};

VESPERENGINE_NAMESPACE_END