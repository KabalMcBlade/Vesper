#include "Systems/base_render_system.h"


VESPERENGINE_NAMESPACE_BEGIN

BaseRenderSystem::BaseRenderSystem(Device& _device)
	: CoreRenderSystem(_device)
{
}

void BaseRenderSystem::Update(const FrameInfo& _frameInfo)
{
	UpdateFrame(_frameInfo);
}

void BaseRenderSystem::Render(const FrameInfo& _frameInfo)
{
	m_pipeline->Bind(_frameInfo.CommandBuffer);

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

	RenderFrame(_frameInfo);
}

VESPERENGINE_NAMESPACE_END
