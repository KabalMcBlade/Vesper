// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\brdf_lut_generation_system.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Systems/base_render_system.h"

#include "Components/graphics_components.h"

#include "vma/vk_mem_alloc.h"


VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;
class Pipeline;
class Buffer;

class VESPERENGINE_API BRDFLUTGenerationSystem : public BaseRenderSystem
{
public:
	BRDFLUTGenerationSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass);
	virtual ~BRDFLUTGenerationSystem();

	BRDFLUTGenerationSystem(const BRDFLUTGenerationSystem&) = delete;
	BRDFLUTGenerationSystem& operator=(const BRDFLUTGenerationSystem&) = delete;

public:
	void Generate(VkCommandBuffer _commandBuffer, uint32 _width, uint32 _height);

private:
	void CreatePipeline(VkRenderPass _renderPass);
	
private:
	VesperApp& m_app;
	VertexBufferComponent m_quadVertexBufferComponent;
	std::unique_ptr<Pipeline> m_pipeline;
	std::unique_ptr<Buffer> m_buffer;
};

VESPERENGINE_NAMESPACE_END
