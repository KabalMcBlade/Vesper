// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\master_render_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/master_render_system.h"


VESPERENGINE_NAMESPACE_BEGIN

MasterRenderSystem::MasterRenderSystem(Device& _device, VkDescriptorSetLayout _globalDescriptorSetLayout, VkDescriptorSetLayout _groupDescriptorSetLayout)
	: CoreRenderSystem(_device)
{
	// IS ONLY FOR TESTING!!!
	// THIS IS HERE BECAUSE THERE IS ONE IN THE OPAQUE RENDERER AND IT IS IN NEED FOR COMMON PIPELINE LAYOUT!
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(glm::vec3);

	m_pushConstants.push_back(pushConstantRange);

	CreatePipelineLayout(std::vector<VkDescriptorSetLayout>
		{ _globalDescriptorSetLayout, _groupDescriptorSetLayout }
	);
}

void MasterRenderSystem::BindGlobalDescriptor(const FrameInfo& _frameInfo)
{
	// GLOBAL DESCRIPTOR: SCENE
	vkCmdBindDescriptorSets(
		_frameInfo.CommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,	// for now is only graphics, in future we want also the compute version
		m_pipelineLayout,
		0,
		1,
		&_frameInfo.GlobalDescriptorSet,
		0,
		nullptr
	);
}

void MasterRenderSystem::Cleanup()
{

}

VESPERENGINE_NAMESPACE_END
