#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/model_data.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"

#include "ECS/ecs.h"

#include <vector>
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL ModelSystem
{
public:
	ModelSystem(Device& _device);
	~ModelSystem() = default;

	ModelSystem(const ModelSystem&) = delete;
	ModelSystem& operator=(const ModelSystem&) = delete;

public:
	void LoadModel(ecs::Entity _entity, std::shared_ptr<ModelData> _data) const;
	void UnloadModel(ecs::Entity _entity) const;
	void UnloadModels() const;

private:
	VertexBufferComponent CreateVertexBuffers(const std::vector<Vertex>& _vertices) const;
	IndexBufferComponent CreateIndexBuffer(const std::vector<uint32>& _indices) const;
	VertexBufferComponent CreateVertexBuffersWithStagingBuffer(const std::vector<Vertex>& _vertices) const;
	IndexBufferComponent CreateIndexBufferWithStagingBuffer(const std::vector<uint32>& _indices) const;

private:
	Device& m_device;
	std::unique_ptr<Buffer> m_buffer;
};

VESPERENGINE_NAMESPACE_END
