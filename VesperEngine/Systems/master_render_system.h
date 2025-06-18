// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\master_render_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/descriptors.h"

#include "Systems/base_render_system.h"

#include "vulkan/vulkan.h"

#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

class TextureSystem;
class MaterialSystem;
class DescriptorSetLayout;
class Buffer;
class Renderer;
class LightSystem;

struct TextureData;
struct CameraTransformComponent;
struct CameraComponent;
struct FrameInfo;
struct LightsUBO;
struct BufferComponent;

/**
 * Vulkan Canonical Viewing Volume
 * x is in range of -1 to 1
 * y is in range of -1 to 1
 * z is in range of 0 to 1
 * This is right hand coordinate system
 * Not that positive x is pointing right, positive y is down and positive z is point in to the screen (from screen away)
 * 
 */

class VESPERENGINE_API MasterRenderSystem : public BaseRenderSystem
{
public:
	static constexpr uint32 kGlobalBindingSceneIndex = 0u;
	static constexpr uint32 kGlobalBindingLightsIndex = 1u;
	static constexpr uint32 kGlobalBindingIrradianceIndex = 2u;
	static constexpr uint32 kGlobalBindingPrefilteredEnvIndex = 3u;
	static constexpr uint32 kGlobalBindingBrdfLutIndex = 4u;

	// bindless settings
	static constexpr uint32 kBindlessBindingTexturesIndex = 0u;
	static constexpr uint32 kBindlessBindingBuffersIndex = 1u;

	static constexpr uint32 kMaxBindlessTextures = 1024;
	static constexpr uint32 kMaxBindlessBuffers = 1024;

public:
	MasterRenderSystem(Device& _device, Renderer& _renderer, LightSystem& _lightSystem);
	virtual ~MasterRenderSystem() = default;

	MasterRenderSystem(const MasterRenderSystem&) = delete;
	MasterRenderSystem& operator=(const MasterRenderSystem&) = delete;

public:
	VESPERENGINE_INLINE VkDescriptorSetLayout GetGlobalDescriptorSetLayout() const { return m_globalSetLayout->GetDescriptorSetLayout(); }
	VESPERENGINE_INLINE VkDescriptorSet GetGlobalDescriptorSet(const int32 _frameIndex) const { return m_globalDescriptorSets[_frameIndex]; }
	VESPERENGINE_INLINE VkDescriptorSet GetBindlessBindingDescriptorSet(const int32 _frameIndex) const { return m_bindlessBindingDescriptorSets[_frameIndex]; }
	
	// this might be present or not
	VESPERENGINE_INLINE VkDescriptorSetLayout GetBindlessBindingDescriptorSetLayout() const 
	{ 
		return m_device.IsBindlessResourcesSupported() ? m_bindlesslSetLayout->GetDescriptorSetLayout() : VK_NULL_HANDLE; 
	}

public:
	// Call this at the beginning, but after all the constructors of all the system is done
	void Initialize(TextureSystem& _textureSystem, MaterialSystem& _materialSystem,
		std::shared_ptr<TextureData> _irradianceMap,
		std::shared_ptr<TextureData> _prefilteredEnvMap,
		std::shared_ptr<TextureData> _brdfLut);
	// Call this within the update/render loop. Does not need to be between the swapchain, and need to be done before the render and the BindGlobalDescriptor
	void UpdateScene(const FrameInfo& _frameInfo, const CameraComponent& _cameraComponen, const CameraTransformComponent& _cameraTransform);
	// Call this after the UpdateScene, but before every other Render from every other system, this is the global descriptor binding point
	void BindGlobalDescriptor(const FrameInfo& _frameInfo);
	// Call at the end or at destruction time, anyway after the game loop is done.
	void Cleanup();

private:
	Renderer& m_renderer;
	LightSystem& m_lightSystem;

	std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
	std::unique_ptr<DescriptorSetLayout> m_bindlesslSetLayout;

	std::vector<BufferComponent> m_globalSceneUboBuffers;
	std::vector<BufferComponent> m_globalLightsUboBuffers;

	std::vector<VkDescriptorSet> m_globalDescriptorSets;
	std::vector<VkDescriptorSet> m_bindlessBindingDescriptorSets;

	VkDescriptorImageInfo m_irradianceInfo{};
	VkDescriptorImageInfo m_prefilteredEnvInfo{};
	VkDescriptorImageInfo m_brdfLutInfo{};

    std::unique_ptr<Buffer> m_buffer;
};

VESPERENGINE_NAMESPACE_END
