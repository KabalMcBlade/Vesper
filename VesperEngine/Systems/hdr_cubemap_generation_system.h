// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\hdr_cubemap_generation_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Systems/base_render_system.h"

#include "vulkan/vulkan.h"

#include <memory>
#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;
class DescriptorPool;
class DescriptorSetLayout;
class Buffer;
class Pipeline;
struct BufferComponent;

enum class HDRProjectionType : uint8
{
	Equirectangular,
	Cube,
	Hemisphere,
	Parabolic,
	LatLongCubemap
};

class VESPERENGINE_API HDRCubemapGenerationSystem : public BaseRenderSystem
{
public:
	static HDRProjectionType DetectHDRProjectionType(int32 _width, int32 _height);

	static bool IsHemisphere(int32 _width, int32 _height);
	static bool IsCubeCross(int32 _width, int32 _height);
	static bool IsEquirectangular(int32 _width, int32 _height);
	static bool IsLatLongCubemap(int32 _width, int32 _height);
	static bool IsParabolic(int32 _width, int32 _height);

private:
	static bool CheckForHemispheres(int32 _width, int32 _height);
	static bool CheckForFaceLayout(int32 _width, int32 _height);

public:
	HDRCubemapGenerationSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass, uint32 _width, uint32 _height);
	virtual ~HDRCubemapGenerationSystem();

	HDRCubemapGenerationSystem(const HDRCubemapGenerationSystem&) = delete;
	HDRCubemapGenerationSystem& operator=(const HDRCubemapGenerationSystem&) = delete;

public:
	void Generate(VkCommandBuffer _commandBuffer, VkImageView _hdrImageView, VkSampler _hdrImageSampler, uint32 _cubemapSize, uint32 _faceIndex);

private:
	void CreatePipeline(VkRenderPass _renderPass);

private:
	VesperApp& m_app;
	std::unique_ptr<DescriptorPool> m_descriptorPool;
	std::vector<BufferComponent> m_quadVertexBufferComponent;
	std::unique_ptr<DescriptorSetLayout> m_hdrGenerationSetLayout;
	std::unique_ptr<Pipeline> m_pipeline;
	std::unique_ptr<Buffer> m_buffer;
	HDRProjectionType m_projectionType;
};

VESPERENGINE_NAMESPACE_END
