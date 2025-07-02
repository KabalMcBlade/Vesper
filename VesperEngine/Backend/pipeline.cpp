// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\pipeline.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Backend/pipeline.h"
#include "Backend/model_data.h"
#include "Backend/device.h"

#include <stdexcept>
#include <fstream>


VESPERENGINE_NAMESPACE_BEGIN

struct SpecializationData
{
	VkSpecializationInfo SpecializationInfo{};
	std::vector<VkSpecializationMapEntry> MapEntries;
	std::vector<uint8_t> DataBuffer;
};

void Pipeline::DefaultPipelineConfiguration(PipelineConfigInfo& _outConfigInfo)
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. INPUT ASSEMBLER STAGE AND VERTEX SHADER (gl_Position)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	_outConfigInfo.InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	_outConfigInfo.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// if VK_TRUE, while using strip variant of topology, we can break up the strip inserting a special index value into an index buffer (special index: 0xFFFF or 0xFFFFFFFF)
	_outConfigInfo.InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	_outConfigInfo.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	_outConfigInfo.ViewportInfo.viewportCount = 1;
	_outConfigInfo.ViewportInfo.pViewports = nullptr;
	_outConfigInfo.ViewportInfo.scissorCount = 1;
	_outConfigInfo.ViewportInfo.pScissors = nullptr;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. RASTERIZATION STAGE AND FRAGMENT SHADER
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// This stage breaks up the geometry into fragments for each pixel or triangle overlaps
	_outConfigInfo.RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// if VK_TRUE, force the Z component of gl_Position to be between 0 and 1, so values less than 0 are clamped to 0
	// and values greater than 1 are clamped to 1
	// We don't want this, because if it is something negative, usually means is behind the camera, and if is greater than 1 means is far away from the camera.
	_outConfigInfo.RasterizationInfo.depthClampEnable = VK_FALSE;
	// Discard all primitives before rasterization, so if we do not continue in the next stage of the pipeline, we can set to VK_TRUE, but if we are using
	// the all stages, like in normal situation, this has to be VK_FALSE
	_outConfigInfo.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	// how do we want to draw the triangle, just corners, edges, the whole geometry, etc... 
	_outConfigInfo.RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	_outConfigInfo.RasterizationInfo.lineWidth = 1.0f;
	// depending by the facing, we can define to cull away what is back, front, both or as by default, nothing and render front and back
	_outConfigInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	// winding order, providing a vertex 0, the 1 and the 2 are in this order, counter clockwise at the moment, we can swap the order.
	// using this we can determine which face of the triangle we are seeing.
	_outConfigInfo.RasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	// the depths bias setting can be used to alter depth values, by a constant value or by a factor of the fragment's slope
	_outConfigInfo.RasterizationInfo.depthBiasEnable = VK_FALSE;
	_outConfigInfo.RasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
	_outConfigInfo.RasterizationInfo.depthBiasClamp = 0.0f;           // Optional
	_outConfigInfo.RasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

	// how the rasterizer handle the edges of the geometry
	// if it is disabled, a fragment is considered either complete in or completely out of a triangle
	// based on where the pixel center is. This usually result is some ugly visual artifacts, if disable, that is, aliasing.
	_outConfigInfo.MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	_outConfigInfo.MultisampleInfo.sampleShadingEnable = VK_FALSE;
	_outConfigInfo.MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	_outConfigInfo.MultisampleInfo.minSampleShading = 1.0f;           // Optional
	_outConfigInfo.MultisampleInfo.pSampleMask = nullptr;             // Optional
	_outConfigInfo.MultisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
	_outConfigInfo.MultisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. COLOR BLENDING STAGE AND FRAME BUFFER
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	_outConfigInfo.ColorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	_outConfigInfo.ColorBlendAttachment.blendEnable = VK_FALSE;
	_outConfigInfo.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	_outConfigInfo.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	_outConfigInfo.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
	_outConfigInfo.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	_outConfigInfo.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	_outConfigInfo.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

	_outConfigInfo.ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	_outConfigInfo.ColorBlendInfo.logicOpEnable = VK_FALSE;
	_outConfigInfo.ColorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
	_outConfigInfo.ColorBlendInfo.attachmentCount = 1;
	_outConfigInfo.ColorBlendInfo.pAttachments = &_outConfigInfo.ColorBlendAttachment;
	_outConfigInfo.ColorBlendInfo.blendConstants[0] = 0.0f;  // Optional
	_outConfigInfo.ColorBlendInfo.blendConstants[1] = 0.0f;  // Optional
	_outConfigInfo.ColorBlendInfo.blendConstants[2] = 0.0f;  // Optional
	_outConfigInfo.ColorBlendInfo.blendConstants[3] = 0.0f;  // Optional

	// clearer color in the depth buffer is away, while darker color in the depth is closer
	_outConfigInfo.DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	_outConfigInfo.DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	_outConfigInfo.DepthStencilInfo.minDepthBounds = 0.0f;  // Optional
	_outConfigInfo.DepthStencilInfo.maxDepthBounds = 1.0f;  // Optional
	_outConfigInfo.DepthStencilInfo.stencilTestEnable = VK_FALSE;
	_outConfigInfo.DepthStencilInfo.front = {};  // Optional
	_outConfigInfo.DepthStencilInfo.back = {};   // Optional

	// This tell the pipeline to expect to have dynamic viewport and dynamic scissor been provided later
	_outConfigInfo.DynamicStateEnables = 
	{ 
		// Viewport describer the transformation between the pipeline's output and target image
		// The viewport is in continuous range of (-1,-1) to (1,1), starting from top left going to bottom right. The middle is (0,0)
		// The framebuffer, the target image, start from (0,0) and go to (width, height), from top left to bottom right.
		// The Viewport is dealing with this transformation
		// So the Viewport tells to the pipeline how we want to transform the gl_Position values (the one in the vertex shader) to the output image
		VK_DYNAMIC_STATE_VIEWPORT, 

		// Scissor is like Viewport, but instead "squash" the result image (Viewport.width/height if changed keep the image but squashed), it will cut it.
		// Any pixels outside of the Scissor rectangle (offset + extent) will be discarded.
		VK_DYNAMIC_STATE_SCISSOR,

		// Allow changing culling mode dynamically
		VK_DYNAMIC_STATE_CULL_MODE,

		// Allow changing front face dynamically
		VK_DYNAMIC_STATE_FRONT_FACE
	};
	_outConfigInfo.DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	_outConfigInfo.DynamicStateInfo.pDynamicStates = _outConfigInfo.DynamicStateEnables.data();
	_outConfigInfo.DynamicStateInfo.dynamicStateCount = static_cast<uint32>(_outConfigInfo.DynamicStateEnables.size());
	_outConfigInfo.DynamicStateInfo.flags = 0;

	_outConfigInfo.BindingDescriptions = Vertex::GetBindingDescriptions();
	_outConfigInfo.AttributeDescriptions = Vertex::GetAttributeDescriptions();
}

// Default pipeline for solid objects.
void Pipeline::OpaquePipelineConfiguration(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.RasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	_outConfigInfo.ColorBlendAttachment.blendEnable = VK_FALSE;  // No blending
}

// Renders objects with transparency.
void Pipeline::TransparentPipelineConfiguration(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;  // Avoid depth overwrites
	_outConfigInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;

	_outConfigInfo.ColorBlendAttachment.blendEnable = VK_TRUE;
	_outConfigInfo.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	_outConfigInfo.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	_outConfigInfo.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	_outConfigInfo.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	_outConfigInfo.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	_outConfigInfo.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

// Renders depth information to a shadow map.
void Pipeline::ShadowPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;

	_outConfigInfo.ColorBlendAttachment.colorWriteMask = 0;  // Disable color writes
	_outConfigInfo.ColorBlendAttachment.blendEnable = VK_FALSE;
	_outConfigInfo.RasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;  // Cull front faces for directional lights
}

// Applies post-process effects to the rendered image.
void Pipeline::PostProcessingPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_FALSE;  // No depth testing
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;

	_outConfigInfo.ColorBlendAttachment.blendEnable = VK_FALSE;  // No blending
}

// Renders a cube map for the skybox.
void Pipeline::SkyboxPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;  // Prevent depth overwrite
	_outConfigInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	
	// Render the skybox from inside the cube
	_outConfigInfo.RasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;  // Cull front faces
	// Cube indices are defined clockwise from the outside, so treat
	// counter-clockwise as front-facing to discard the outer surface
	_outConfigInfo.RasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	// Only position attribute is required for the cube
	_outConfigInfo.AttributeDescriptions.clear();
	_outConfigInfo.AttributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position) });
}

// Renders 2D elements like UI or HUD.
void Pipeline::UIPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_FALSE;  // No depth testing
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;

	_outConfigInfo.ColorBlendAttachment.blendEnable = VK_TRUE;
	_outConfigInfo.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	_outConfigInfo.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	_outConfigInfo.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	_outConfigInfo.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	_outConfigInfo.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	_outConfigInfo.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

// Outputs GBuffer for lighting calculations in a separate pass.
void Pipeline::DeferredShadingPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;

	// Set up MRT for GBuffer
	_outConfigInfo.ColorBlendAttachment.blendEnable = VK_FALSE;
	_outConfigInfo.ColorBlendInfo.attachmentCount = 3;  // Example: Albedo, Normals, and Depth
	// Add configuration for multiple render targets as needed
}

// Optimized for handling large numbers of lights using tiled or clustered rendering.
void Pipeline::ForwardPlusPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;

	// Adjust the configuration to include light culling data
	_outConfigInfo.DynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	_outConfigInfo.DynamicStateInfo.dynamicStateCount = static_cast<uint32>(_outConfigInfo.DynamicStateEnables.size());
}

// Handles ray - traced lighting, shadows, and reflections.
void Pipeline::RayTracingPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_FALSE;  // Usually integrated with rasterization
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;

	// Specific settings for ray tracing pipelines depend on Vulkan RT extensions
}

// Renders volumetric effects like fog, light shafts, and clouds.
void Pipeline::VolumetricPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_FALSE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;

	_outConfigInfo.ColorBlendAttachment.blendEnable = VK_TRUE;
	_outConfigInfo.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	_outConfigInfo.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	_outConfigInfo.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
}

// Handles GPU compute tasks for physics, particles, or post-processing.
void Pipeline::ComputePipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	// Compute pipelines do not use rasterization setting, so is fine the default
}

// Visualizes geometry in wireframe mode.
void Pipeline::WireframePipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.RasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
	_outConfigInfo.RasterizationInfo.lineWidth = 1.0f;
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;
}

// Renders bounding boxes for objects.
void Pipeline::BoundingBoxPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.RasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;  // Ensure transparency for debugging
}

// Visualizes normals by color-coding the geometry.
void Pipeline::NormalsVisualizationPipelineConfig(PipelineConfigInfo& _outConfigInfo)
{
	Pipeline::DefaultPipelineConfiguration(_outConfigInfo);
	_outConfigInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	_outConfigInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;

	// Use custom shaders to color-code normals
}


Pipeline::Pipeline(Device& _device, const std::vector<ShaderInfo>& _shadersInfo, const PipelineConfigInfo& _configInfo)
 : m_device{_device}
{
	CreateGraphicsPipeline(_shadersInfo, _configInfo);
}

Pipeline::~Pipeline()
{
	for (const VkShaderModule& shaderModule : m_shaderModules)
	{
		vkDestroyShaderModule(m_device.GetDevice(), shaderModule, nullptr);
	}
	vkDestroyPipeline(m_device.GetDevice(), m_graphicPipeline, nullptr);
}

void Pipeline::Bind(VkCommandBuffer _commandBuffer)
{
	// VK_PIPELINE_BIND_POINT_GRAPHICS signal is a graphic pipeline (other are compute and ray tracing)
	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline);

	// TODO: This break Skybox!
	// fallback, to avoid validation error, since is expecting culling to be dynamic (for now)
//	vkCmdSetCullModeEXT(_commandBuffer, VK_CULL_MODE_BACK_BIT);
//	vkCmdSetFrontFaceEXT(_commandBuffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	//vkCmdSetDepthTestEnableEXT(_commandBuffer, VK_TRUE);          // if depth test is dynamic
	//vkCmdSetDepthWriteEnableEXT(_commandBuffer, VK_TRUE);         // if write is dynamic
	//vkCmdSetBlendConstants(_commandBuffer, blendConstants);       // if blending is dynamic
}

std::vector<int8> Pipeline::ReadFile(const std::string& _filepath)
{
	std::ifstream file { _filepath, std::ios::ate | std::ios::binary };

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file: " + _filepath + "!");
	}

	std::size_t fileSize = static_cast<std::size_t>(file.tellg());
	std::vector<int8> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkShaderStageFlagBits Pipeline::ConvertShaderTypeToShaderFlag(ShaderType _type) const
{
	switch (_type)
	{
	case vesper::ShaderType::Vertex:					return VK_SHADER_STAGE_VERTEX_BIT;
	case vesper::ShaderType::TessellationEvaluation:	return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case vesper::ShaderType::TessellationControl:		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case vesper::ShaderType::Geometry:					return VK_SHADER_STAGE_GEOMETRY_BIT;
	case vesper::ShaderType::Fragment:					return VK_SHADER_STAGE_FRAGMENT_BIT;
	default:											return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}
}


void Pipeline::CreateGraphicsPipeline(const std::vector<ShaderInfo>& _shadersInfo, const PipelineConfigInfo& _configInfo)
{
	assertMsgReturnVoid(_configInfo.PipelineLayout != VK_NULL_HANDLE, "Cannot create graphic pipeline: No PipelineLayout passed in _configInfo");
	assertMsgReturnVoid(_configInfo.RenderPass != VK_NULL_HANDLE, "Cannot create graphic pipeline: No RenderPass passed in _configInfo");

	const int8 count = static_cast<int8>(_shadersInfo.size());

	m_shaderModules.resize(count);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(count);

	// specialization constants
	std::vector<SpecializationData> specializationDatas(count);

	for (int8 i = 0; i < count; ++i)
	{
		const ShaderInfo& shaderInfo = _shadersInfo[i];

		auto shader = ReadFile(shaderInfo.Filepath);

		CreateShaderModule(shader, &m_shaderModules[i]);

		shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[i].stage = ConvertShaderTypeToShaderFlag(shaderInfo.Type);
		shaderStages[i].module = m_shaderModules[i];
		shaderStages[i].pName = "main";
		shaderStages[i].flags = 0;
		shaderStages[i].pNext = nullptr;

		if (!shaderInfo.SpecializationConstants.empty())
		{
			SpecializationData& specData = specializationDatas[i];

			specData.MapEntries.resize(shaderInfo.SpecializationConstants.size());

			uint32 offset = 0;
			specData.DataBuffer.clear();

			for (size_t j = 0; j < shaderInfo.SpecializationConstants.size(); ++j)
			{
				const auto& specConst = shaderInfo.SpecializationConstants[j];

				VkSpecializationMapEntry& entry = specData.MapEntries[j];
				entry.constantID = specConst.ID;
				entry.offset = offset;
				entry.size = specConst.Value.size();

				specData.DataBuffer.insert(specData.DataBuffer.end(), specConst.Value.begin(), specConst.Value.end());
				offset += static_cast<uint32>(specConst.Value.size());
			}

			specData.SpecializationInfo.mapEntryCount = static_cast<uint32>(specData.MapEntries.size());
			specData.SpecializationInfo.pMapEntries = specData.MapEntries.data();
			specData.SpecializationInfo.dataSize = specData.DataBuffer.size();
			specData.SpecializationInfo.pData = specData.DataBuffer.data();

			shaderStages[i].pSpecializationInfo = &specData.SpecializationInfo;
		}
		else
		{
			shaderStages[i].pSpecializationInfo = nullptr;
		}
	}
	
	auto& bindingDescriptions = _configInfo.BindingDescriptions;
	auto& attributeDescriptions = _configInfo.AttributeDescriptions;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32>(attributeDescriptions.size());
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32>(bindingDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &_configInfo.InputAssemblyInfo;
	pipelineInfo.pViewportState = &_configInfo.ViewportInfo;
	pipelineInfo.pRasterizationState = &_configInfo.RasterizationInfo;
	pipelineInfo.pMultisampleState = &_configInfo.MultisampleInfo;

	pipelineInfo.pColorBlendState = &_configInfo.ColorBlendInfo;
	pipelineInfo.pDepthStencilState = &_configInfo.DepthStencilInfo;
	pipelineInfo.pDynamicState = &_configInfo.DynamicStateInfo;

	pipelineInfo.layout = _configInfo.PipelineLayout;
	pipelineInfo.renderPass = _configInfo.RenderPass;
	pipelineInfo.subpass = _configInfo.Subpass;

	// Tweak and use this value when try to optimize performance.
	// Can be less expensive for a GPU to create a new graphic pipeline by deriving from an existing one
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(m_device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create the graphic pipeline");
	}
}

void Pipeline::CreateShaderModule(const std::vector<int8>& _buffer, VkShaderModule* _shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = _buffer.size();
	createInfo.pCode =  reinterpret_cast<const uint32*>(_buffer.data());

	if (vkCreateShaderModule(m_device.GetDevice(), &createInfo, nullptr, _shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
}

VESPERENGINE_NAMESPACE_END
