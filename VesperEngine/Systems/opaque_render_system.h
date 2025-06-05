// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\opaque_render_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Systems/phong_render_system.h"

#include "vulkan/vulkan.h"

#include <memory>
#include <vector>

// USING BINDLESS
// set 0: global descriptor set layout
// set 1: bindless textures and buffer descriptor set layout
// set 2: entity descriptor set layout
// set 3: material descriptor set layout
// 
// OR NORMAL BINDING
// set 0: global descriptor set layout
// set 1: entity descriptor set layout
// set 2: material descriptor set layout

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;
class Renderer;
class Pipeline;
class DescriptorSetLayout;
class Buffer;

struct FrameInfo;

class VESPERENGINE_API OpaqueRenderSystem final : public PhongRenderSystem
{
public:
        OpaqueRenderSystem(VesperApp& _app, Device& _device, Renderer& _renderer,
                VkDescriptorSetLayout _globalDescriptorSetLayout,
                VkDescriptorSetLayout _entityDescriptorSetLayout,
                VkDescriptorSetLayout _bindlessBindingDescriptorSetLayout = VK_NULL_HANDLE);
        ~OpaqueRenderSystem();

	OpaqueRenderSystem(const OpaqueRenderSystem&) = delete;
	OpaqueRenderSystem& operator=(const OpaqueRenderSystem&) = delete;

public:
	// Call at initialization time
	void MaterialBinding();
	// Call between begin/end frame, do not need to be called between begin/end swap chain render pass. But need to be called before Render
	void Update(const FrameInfo& _frameInfo);
	// Call between begin swap chain render pass/end swap chain render pass.
	void Render(const FrameInfo& _frameInfo);
	// Call at the end or at destruction time, anyway after the game loop is done.
        void Cleanup();

private:
        void CreatePipeline(VkRenderPass _renderPass);
};

VESPERENGINE_NAMESPACE_END
