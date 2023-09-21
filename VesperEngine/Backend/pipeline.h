#pragma once

#include "Core/core_defines.h"
#include "Backend/device.h"

#include <vector>
#include <string>


VESPERENGINE_NAMESPACE_BEGIN

struct VESPERENGINE_DLL PipelineConfigInfo
{
	PipelineConfigInfo() = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	VkPipelineViewportStateCreateInfo ViewportInfo;
	VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo RasterizationInfo;
	VkPipelineMultisampleStateCreateInfo MultisampleInfo;
	VkPipelineColorBlendAttachmentState ColorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo ColorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo DepthStencilInfo;

	std::vector<VkDynamicState> DynamicStateEnables;
	VkPipelineDynamicStateCreateInfo DynamicStateInfo;

	VkPipelineLayout PipelineLayout = nullptr;
	VkRenderPass RenderPass = nullptr;
	uint32 Subpass = 0u;
};

class VESPERENGINE_DLL Pipeline final
{
public:
	static void DefaultPipelineConfiguration(PipelineConfigInfo& _outConfigInfo);

public:
	Pipeline(Device& _device, const std::string& _filepath_vert, const std::string& _filepath_frag, const PipelineConfigInfo& _configInfo);
	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

public:
	void Bind(VkCommandBuffer _commandBuffer);

private:
	static std::vector<int8> ReadFile(const std::string& _filepath);

private:
	void CreateGraphicsPipeline(const std::string& _filepath_vert, const std::string& _filepath_frag, const PipelineConfigInfo& _configInfo);

	void CreateShaderModule(const std::vector<int8>& _buffer, VkShaderModule* _shaderModule);

private:
	Device& m_device;

	VkPipeline m_graphicPipeline;
	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;
};

VESPERENGINE_NAMESPACE_END
