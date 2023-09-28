#include "pch.h"
#include "Backend/pipeline.h"

#include "Backend/model_data.h"

#include <stdexcept>


VESPERENGINE_NAMESPACE_BEGIN

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
	// winding order, providing a vertex 0, the 1 and the 2 are in this order, clockwise at the moment, we can swap the order.
	// using this we can determine which face of the triangle we are seeing.
	_outConfigInfo.RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
		VK_DYNAMIC_STATE_SCISSOR 
	};
	_outConfigInfo.DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	_outConfigInfo.DynamicStateInfo.pDynamicStates = _outConfigInfo.DynamicStateEnables.data();
	_outConfigInfo.DynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(_outConfigInfo.DynamicStateEnables.size());
	_outConfigInfo.DynamicStateInfo.flags = 0;
}

Pipeline::Pipeline(Device& _device, const std::string& _filepath_vert, const std::string& _filepath_frag, const PipelineConfigInfo& _configInfo)
 : m_device{_device}
{
	CreateGraphicsPipeline(_filepath_vert, _filepath_frag, _configInfo);
}

Pipeline::~Pipeline()
{
	vkDestroyShaderModule(m_device.GetDevice(), m_vertexShaderModule, nullptr);
	vkDestroyShaderModule(m_device.GetDevice(), m_fragmentShaderModule, nullptr);
	vkDestroyPipeline(m_device.GetDevice(), m_graphicPipeline, nullptr);
}

void Pipeline::Bind(VkCommandBuffer _commandBuffer)
{
	// VK_PIPELINE_BIND_POINT_GRAPHICS signal is a graphic pipeline (other are compute and ray tracing)
	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline);
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

void Pipeline::CreateGraphicsPipeline(const std::string& _filepath_vert, const std::string& _filepath_frag, const PipelineConfigInfo& _configInfo)
{
	assertMsgReturnVoid(_configInfo.PipelineLayout != VK_NULL_HANDLE, "Cannot create graphic pipeline: No PipelineLayout passed in _configInfo");
	assertMsgReturnVoid(_configInfo.RenderPass != VK_NULL_HANDLE, "Cannot create graphic pipeline: No RenderPass passed in _configInfo");

	auto vertex = ReadFile(_filepath_vert);
	auto fragment = ReadFile(_filepath_frag);

#ifdef _DEBUG
	std::cout << "Vertex Shader Code Size = " << vertex.size() << std::endl;
	std::cout << "Fragment Shader Code Size = " << fragment.size() << std::endl;
#endif

	CreateShaderModule(vertex, &m_vertexShaderModule);
	CreateShaderModule(fragment, &m_fragmentShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2];

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = m_vertexShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = m_fragmentShaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	
	auto attributeDescription = Vertex::GetAttributeDescriptions();
	auto bindingDescription = Vertex::GetBindingDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32>(attributeDescription.size());
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32>(bindingDescription.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2; // we have only vertex and fragment shader for now, so 2
	pipelineInfo.pStages = shaderStages;
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