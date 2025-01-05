#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/model_data.h"

#include "Systems/core_render_system.h"

#include "App/vesper_app.h"


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API BRDFLUTGenerationSystem : public CoreRenderSystem
{
public:
	BRDFLUTGenerationSystem(VesperApp& _app, Device& _device, VkRenderPass _renderPass);
	virtual ~BRDFLUTGenerationSystem();

	BRDFLUTGenerationSystem(const BRDFLUTGenerationSystem&) = delete;
	BRDFLUTGenerationSystem& operator=(const BRDFLUTGenerationSystem&) = delete;

public:
	void Generate(VkCommandBuffer _commandBuffer, uint32 _width, uint32 _height);

protected:
	virtual void SetupPipeline(PipelineConfigInfo& _pipelineConfig) override;
	
private:
	VesperApp& m_app;
	VertexBufferComponent m_quadVertexBufferComponent;
	std::unique_ptr<Buffer> m_buffer;
};

VESPERENGINE_NAMESPACE_END
