#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"

#include "Backend/device.h"
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
	void CreateVertexBuffers(VertexBufferComponent& _vertexBufferComponent, const std::vector<Vertex>& _vertices) const;
	void CreateIndexBuffer(IndexBufferComponent& _indexBufferComponent, const std::vector<uint32>& _indices) const;
	void CreateVertexBuffersWithStagingBuffer(VertexBufferComponent& _vertexBufferComponent, const std::vector<Vertex>& _vertices) const;
	void CreateIndexBufferWithStagingBuffer(IndexBufferComponent& _indexBufferComponent, const std::vector<uint32>& _indices) const;

private:
	Device& m_device;
};

VESPERENGINE_NAMESPACE_END
