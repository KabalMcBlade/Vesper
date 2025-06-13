// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\CustomOpaqueRenderSystem.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vesper.h"

VESPERENGINE_USING_NAMESPACE

class CustomOpaqueRenderSystem final : public OpaqueRenderSystem
{
public:
    CustomOpaqueRenderSystem(VesperApp& app, Device& device, Renderer& renderer,
        VkDescriptorSetLayout globalDescriptorSetLayout,
        VkDescriptorSetLayout entityDescriptorSetLayout,
        VkDescriptorSetLayout bindlessBindingDescriptorSetLayout = VK_NULL_HANDLE);
    ~CustomOpaqueRenderSystem() override;

public:
    void CreatePipeline(VkRenderPass renderPass) override;

protected:
    void PerEntityUpdate(const FrameInfo& _frameInfo, ecs::ComponentManager& _componentManager, const ecs::Entity& _entity) override;
    void PerEntityRender(const FrameInfo& _frameInfo, ecs::ComponentManager& _componentManager, const ecs::Entity& _entity) override;
};