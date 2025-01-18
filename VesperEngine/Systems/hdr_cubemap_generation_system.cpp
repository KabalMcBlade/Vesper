// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\hdr_cubemap_generation_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/hdr_cubemap_generation_system.h"

#include "Core/glm_config.h"

#include "Backend/pipeline.h"
#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/descriptors.h"
#include "Backend/model_data.h"

#include "App/vesper_app.h"
#include "App/config.h"


VESPERENGINE_NAMESPACE_BEGIN

glm::mat4 GetCubemapFaceViewMatrix(uint32 _face)
{
	static const glm::vec3 targets[] = {
		{1, 0, 0},  {-1, 0, 0},  {0, 1, 0},  {0, -1, 0},  {0, 0, 1},  {0, 0, -1},
	};
	static const glm::vec3 ups[] = {
		{0, -1, 0}, {0, -1, 0}, {0, 0, 1},  {0, 0, -1},  {0, -1, 0}, {0, -1, 0},
	};

	return glm::lookAt(glm::vec3(0.0f), targets[_face], ups[_face]);
}

HDRProjectionType HDRCubemapGenerationSystem::DetectHDRProjectionType(int32 _width, int32 _height)
{
	if (IsCubeCross(_width, _height))
	{
		return HDRProjectionType::Cube;
	}
	if (IsParabolic(_width, _height))
	{
		return HDRProjectionType::Parabolic;
	}
	if (IsLatLongCubemap(_width, _height))
	{
		return HDRProjectionType::LatLongCubemap;
	}
	if (IsEquirectangular(_width, _height))
	{
		return HDRProjectionType::Equirectangular;
	}
	if (IsHemisphere(_width, _height))
	{
		return HDRProjectionType::Hemisphere;
	}

	// Default to Equirectangular
	return HDRProjectionType::Equirectangular;
}

bool HDRCubemapGenerationSystem::IsHemisphere(int32 _width, int32 _height)
{
	// A hemisphere texture is usually square.
	return _width == _height;
}

bool HDRCubemapGenerationSystem::IsCubeCross(int32 _width, int32 _height)
{
	bool isVertical = (_width / 3 == _height / 4) && (_width % 3 == 0) && (_height % 4 == 0);
	bool isHorizontal = (_width / 4 == _height / 3) && (_width % 4 == 0) && (_height % 3 == 0);
	return isVertical || isHorizontal;
}

bool HDRCubemapGenerationSystem::IsEquirectangular(int32 _width, int32 _height)
{
	// Equirectangular textures are simple 2:1 aspect ratio
	float aspect = static_cast<float>(_width) / static_cast<float>(_height);
	return std::fabsf(aspect - 2.0f) < 0.00001f;
}

bool HDRCubemapGenerationSystem::IsLatLongCubemap(int32 _width, int32 _height)
{
	// LatLongCubemap might share 2:1 aspect ratio, but typically has identifiable "cubemap face layout"
	// Example heuristic: Check for distinct face-like regions in the image
	return IsEquirectangular(_width, _height) && CheckForFaceLayout(_width, _height);
}

bool HDRCubemapGenerationSystem::IsParabolic(int32 _width, int32 _height)
{
	// Parabolic textures typically have a 2:1 aspect ratio
	// and contain two vertically stacked hemispheres
	float aspect = static_cast<float>(_width) / static_cast<float>(_height);
	return std::fabsf(aspect - 2.0f) < 0.00001f && CheckForHemispheres(_width, _height);
}

// Helper function to check for two hemispheres
bool HDRCubemapGenerationSystem::CheckForHemispheres(int32 _width, int32 _height)
{
	// Heuristic: Verify that the texture can be split into two equal vertical sections
	int32 hemisphereHeight = _height / 2;

	return (hemisphereHeight * 2 == _height);
}

// Helper function to check if the texture contains cubemap-like sections
bool HDRCubemapGenerationSystem::CheckForFaceLayout(int32 _width, int32 _height)
{
	// For simplicity, assume six "face regions" in a 2:1 aspect ratio
	int32 faceWidth = _width / 3;
	int32 faceHeight = _height / 2;

	// Each "face region" should have equal dimensions
	return (faceWidth * 3 == _width) && (faceHeight * 2 == _height);
}

struct VESPERENGINE_ALIGN16 PushViewProjection
{
	glm::mat4 View;
	glm::mat4 Projection;
	int32 ProjectionType;
};

HDRCubemapGenerationSystem::HDRCubemapGenerationSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass, uint32 _width, uint32 _height)
	: m_app{ _app }
	, BaseRenderSystem(_device)
{
	m_buffer = std::make_unique<Buffer>(m_device);

	m_descriptorPool = DescriptorPool::Builder(m_device)
		.SetMaxSets(8)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 32)
		.Build();

	m_projectionType = DetectHDRProjectionType(_width, _height);

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushViewProjection);

	m_pushConstants.push_back(pushConstantRange);

	m_hdrGenerationSetLayout = DescriptorSetLayout::Builder(m_device)
		.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build();

	CreatePipelineLayout(std::vector<VkDescriptorSetLayout>{ m_hdrGenerationSetLayout->GetDescriptorSetLayout() });
	CreatePipeline(_renderPass);
}

HDRCubemapGenerationSystem::~HDRCubemapGenerationSystem()
{
	for (int32 i = 0; i < m_quadVertexBufferComponent.size(); ++i)
	{
		m_buffer->Destroy(m_quadVertexBufferComponent[i]);
	}
}

void HDRCubemapGenerationSystem::Generate(VkCommandBuffer _commandBuffer, VkImageView _hdrImageView, VkSampler _hdrImageSampler, uint32 _cubemapSize, uint32 _faceIndex)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.sampler = _hdrImageSampler;
	imageInfo.imageView = _hdrImageView;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorSet boundDescriptorSet;
	DescriptorWriter(*m_hdrGenerationSetLayout, *m_descriptorPool)
		.WriteImage(0, &imageInfo)
		.Build(boundDescriptorSet);

	vkCmdBindDescriptorSets(
		_commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0,
		1,
		&boundDescriptorSet,
		0,
		nullptr);

	const uint32 vertexCount = 3;
	const VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;
	const uint32 vertexSize = sizeof(Vertex);

	VertexBufferComponent quadVertexBufferComponent = m_buffer->Create<VertexBufferComponent>(
		vertexSize,
		vertexCount,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	m_pipeline->Bind(_commandBuffer);

	PushViewProjection viewProjection;
	viewProjection.View = GetCubemapFaceViewMatrix(_faceIndex);
	viewProjection.Projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	viewProjection.Projection[1][1] *= -1; // Vulkan uses inverted Y
	viewProjection.ProjectionType = static_cast<int32>(m_projectionType);

	PushConstants(_commandBuffer, 0, &viewProjection);

	Bind(quadVertexBufferComponent, _commandBuffer);
	Draw(quadVertexBufferComponent, _commandBuffer);

	m_quadVertexBufferComponent.push_back(quadVertexBufferComponent);
}

void HDRCubemapGenerationSystem::CreatePipeline(VkRenderPass _renderPass)
{
	assertMsgReturnVoid(m_pipelineLayout != nullptr, "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};

	Pipeline::DefaultPipelineConfiguration(pipelineConfig);

	pipelineConfig.RenderPass = _renderPass;
	pipelineConfig.PipelineLayout = m_pipelineLayout;

	pipelineConfig.DepthStencilInfo.depthTestEnable = VK_FALSE;
	pipelineConfig.RasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;

	m_pipeline = std::make_unique<Pipeline>(
		m_device,
		std::vector{ ShaderInfo{m_app.GetConfig().ShadersPath + "fullscreen.vert.spv", ShaderType::Vertex}, ShaderInfo{m_app.GetConfig().ShadersPath + "hdr_cubemap_shader.frag.spv", ShaderType::Fragment}, },
		pipelineConfig
	);
}

VESPERENGINE_NAMESPACE_END
