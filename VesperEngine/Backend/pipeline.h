#pragma once

#include "Core/core_defines.h"
#include "Backend/device.h"

#include <vector>
#include <string>


VESPERENGINE_NAMESPACE_BEGIN

struct PipelineConfigInfo
{
	PipelineConfigInfo() = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	std::vector<VkVertexInputBindingDescription> BindingDescriptions{};
	std::vector<VkVertexInputAttributeDescription> AttributeDescriptions{};
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

enum class ShaderType : uint8
{
	Vertex = 0,
	TessellationEvaluation,
	TessellationControl,
	Geometry,
	Fragment
};

struct ShaderInfo
{
	ShaderInfo(const std::string& _filepath, const ShaderType _type) : Filepath(_filepath), Type(_type){}

	ShaderInfo() = default;
	ShaderInfo(const PipelineConfigInfo&) = delete;
	ShaderInfo& operator=(const PipelineConfigInfo&) = delete;

	std::string Filepath;
	ShaderType Type;
};

class VESPERENGINE_DLL Pipeline final
{
public:
	static void DefaultPipelineConfiguration(PipelineConfigInfo& _outConfigInfo);

public:
	Pipeline(Device& _device, const std::vector<ShaderInfo>& _shadersInfo, const PipelineConfigInfo& _configInfo);
	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

public:
	void Bind(VkCommandBuffer _commandBuffer);

private:
	static std::vector<int8> ReadFile(const std::string& _filepath);

private:
	VkShaderStageFlagBits ConvertShaderTypeToShaderFlag(ShaderType _type) const;
	void CreateGraphicsPipeline(const std::vector<ShaderInfo>& _shadersInfo, const PipelineConfigInfo& _configInfo);
	void CreateShaderModule(const std::vector<int8>& _buffer, VkShaderModule* _shaderModule);

private:
	Device& m_device;

	VkPipeline m_graphicPipeline;
	std::vector<VkShaderModule> m_shaderModules;
};

VESPERENGINE_NAMESPACE_END
