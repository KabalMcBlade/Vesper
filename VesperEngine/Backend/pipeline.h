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

struct SpecializationConstant 
{
	uint32 ID;					// Specialization constant ID
	std::vector<uint8> Value;	// Raw bytes of the constant value

	template <typename T>
	SpecializationConstant(uint32 _id, const T& _value)
		: ID(_id), Value(sizeof(T))
	{
		std::memcpy(Value.data(), &_value, sizeof(T));
	}
};

struct ShaderInfo
{
	ShaderInfo(const std::string& _filepath, const ShaderType _type) : Filepath(_filepath), Type(_type){}

	ShaderInfo() = default;
	ShaderInfo(const PipelineConfigInfo&) = delete;
	ShaderInfo& operator=(const PipelineConfigInfo&) = delete;

	template <typename T>
	void AddSpecializationConstant(uint32 _id, const T& _value)
	{
		SpecializationConstants.emplace_back(_id, _value);
	}

	std::string Filepath;
	ShaderType Type;
	std::vector<SpecializationConstant> SpecializationConstants;
};

class VESPERENGINE_API Pipeline final
{
public:
	// Base settings for all pipelines
	static void DefaultPipelineConfiguration(PipelineConfigInfo& _outConfigInfo);

	// Core pipelines
	static void OpaquePipelineConfiguration(PipelineConfigInfo& _outConfigInfo);
	static void TransparentPipelineConfiguration(PipelineConfigInfo& _outConfigInfo);
	static void ShadowPipelineConfig(PipelineConfigInfo& _outConfigInfo);
	static void PostProcessingPipelineConfig(PipelineConfigInfo& _outConfigInfo);
	static void SkyboxPipelineConfig(PipelineConfigInfo& _outConfigInfo);
	static void UIPipelineConfig(PipelineConfigInfo& _outConfigInfo);

	// Advance pipelines
	static void DeferredShadingPipelineConfig(PipelineConfigInfo& _outConfigInfo);	// NOT YET IMPLEMENTED!
	static void ForwardPlusPipelineConfig(PipelineConfigInfo& _outConfigInfo);		// NOT YET IMPLEMENTED!
	static void RayTracingPipelineConfig(PipelineConfigInfo& _outConfigInfo);			// NOT YET IMPLEMENTED!
	static void VolumetricPipelineConfig(PipelineConfigInfo& _outConfigInfo);
	static void ComputePipelineConfig(PipelineConfigInfo& _outConfigInfo);

	// Debug pipelines
	static void WireframePipelineConfig(PipelineConfigInfo& _outConfigInfo);
	static void BoundingBoxPipelineConfig(PipelineConfigInfo& _outConfigInfo);
	static void NormalsVisualizationPipelineConfig(PipelineConfigInfo& _outConfigInfo);// CUSTOM SHADER NEED IT!


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
