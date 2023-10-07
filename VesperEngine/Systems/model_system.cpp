#include "pch.h"
#include "model_system.h"

#include "Core/memory_copy.h"
#include "Backend/device.h"

#include "ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

ModelSystem::ModelSystem(Device& _device)
	: m_device{ _device }
{
	m_buffer = std::make_unique<Buffer>(m_device);
}

void ModelSystem::LoadModel(ecs::Entity _entity, std::shared_ptr<ModelData> _data) const
{
	// depending by the data, add the component
	if (_data->IsStatic)
	{
		ecs::ComponentManager::AddComponent<StaticComponent>(_entity);
	}

	if (_data->Vertices.size() > 0)
	{
		ecs::ComponentManager::AddComponent<VertexBufferComponent>(_entity);

		VertexBufferComponent& vertexBuffer = ecs::ComponentManager::GetComponent<VertexBufferComponent>(_entity);

		if(_data->IsStatic)
		{
			CreateVertexBuffersWithStagingBuffer(vertexBuffer, _data->Vertices);
		}
		else
		{
			CreateVertexBuffers(vertexBuffer, _data->Vertices);
		}
	}
	else
	{
		ecs::ComponentManager::AddComponent<NotVertexBufferComponent>(_entity);
	}

	if (_data->Indices.size() > 0)
	{
		ecs::ComponentManager::AddComponent<IndexBufferComponent>(_entity);

		IndexBufferComponent& indexBuffer = ecs::ComponentManager::GetComponent<IndexBufferComponent>(_entity);

		if (_data->IsStatic)
		{
			CreateIndexBufferWithStagingBuffer(indexBuffer, _data->Indices);
		}
		else
		{
			CreateIndexBuffer(indexBuffer, _data->Indices);
		}
	}
	else
	{
		ecs::ComponentManager::AddComponent<NotIndexBufferComponent>(_entity);
	}

	// material later
}

void ModelSystem::UnloadModel(ecs::Entity _entity) const
{
	if (ecs::ComponentManager::HasComponents<VertexBufferComponent>(_entity))
	{
		VertexBufferComponent& vertexBuffer = ecs::ComponentManager::GetComponent<VertexBufferComponent>(_entity);

		m_buffer->Destroy(vertexBuffer);

		ecs::ComponentManager::RemoveComponent<VertexBufferComponent>(_entity);
	}

	if (ecs::ComponentManager::HasComponents<IndexBufferComponent>(_entity))
	{
		IndexBufferComponent& indexBuffer = ecs::ComponentManager::GetComponent<IndexBufferComponent>(_entity);

		m_buffer->Destroy(indexBuffer);

		ecs::ComponentManager::RemoveComponent<IndexBufferComponent>(_entity);
	}

	if (ecs::ComponentManager::HasComponents<NotVertexBufferComponent>(_entity))
	{
		ecs::ComponentManager::RemoveComponent<NotVertexBufferComponent>(_entity);
	}

	if (ecs::ComponentManager::HasComponents<NotIndexBufferComponent>(_entity))
	{
		ecs::ComponentManager::RemoveComponent<NotIndexBufferComponent>(_entity);
	}

	if (ecs::ComponentManager::HasComponents<StaticComponent>(_entity))
	{
		ecs::ComponentManager::RemoveComponent<StaticComponent>(_entity);
	}

	// material later
}

void ModelSystem::UnloadModels() const
{
	for (auto iterator : ecs::IterateEntitiesWithAny<VertexBufferComponent, IndexBufferComponent>())
	{
		UnloadModel(iterator);
	}
}

void ModelSystem::CreateVertexBuffers(VertexBufferComponent& _vertexBufferComponent, const std::vector<Vertex>& _vertices) const
{
	_vertexBufferComponent.Count = static_cast<uint32>(_vertices.size());

	assertMsgReturnVoid(_vertexBufferComponent.Count >= 3, "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertexBufferComponent.Count;
	uint32 vertexSize = sizeof(_vertices[0]);

	m_buffer->Create(
		_vertexBufferComponent,
		vertexSize,
		_vertexBufferComponent.Count,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	void* data;
	m_buffer->Map(_vertexBufferComponent, &data);
	m_buffer->WriteToBuffer(data, (void*)_vertices.data(), static_cast<std::size_t>(bufferSize));
	m_buffer->Unmap(_vertexBufferComponent);
}

void ModelSystem::CreateIndexBuffer(IndexBufferComponent& _indexBufferComponent, const std::vector<uint32>& _indices) const
{
	_indexBufferComponent.Count = static_cast<uint32>(_indices.size());

	VkDeviceSize bufferSize = sizeof(_indices[0]) * _indexBufferComponent.Count;
	uint32 indexSize = sizeof(_indices[0]);

	m_buffer->Create(
		_indexBufferComponent,
		indexSize,
		_indexBufferComponent.Count,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	void* data;
	m_buffer->Map(_indexBufferComponent, &data);
	m_buffer->WriteToBuffer(data, (void*)_indices.data(), static_cast<std::size_t>(bufferSize));
	m_buffer->Unmap(_indexBufferComponent);
}


void ModelSystem::CreateVertexBuffersWithStagingBuffer(VertexBufferComponent& _vertexBufferComponent, const std::vector<Vertex>& _vertices) const
{
	_vertexBufferComponent.Count = static_cast<uint32>(_vertices.size());

	assertMsgReturnVoid(_vertexBufferComponent.Count >= 3, "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertexBufferComponent.Count;
	uint32 vertexSize = sizeof(_vertices[0]);

	BufferComponent stagingBuffer;
	m_buffer->Create(
		stagingBuffer,
		vertexSize,
		_vertexBufferComponent.Count,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	void* data;
	m_buffer->Map(stagingBuffer, &data);
	m_buffer->WriteToBuffer(data, (void*)_vertices.data(), static_cast<std::size_t>(bufferSize));
	m_buffer->Unmap(stagingBuffer);
	
	m_buffer->Create(
		_vertexBufferComponent,
		vertexSize,
		_vertexBufferComponent.Count,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	m_buffer->Copy(stagingBuffer, _vertexBufferComponent, bufferSize);

	m_buffer->Destroy(stagingBuffer);
}

void ModelSystem::CreateIndexBufferWithStagingBuffer(IndexBufferComponent& _indexBufferComponent, const std::vector<uint32>& _indices) const
{
	_indexBufferComponent.Count = static_cast<uint32>(_indices.size());

	VkDeviceSize bufferSize = sizeof(_indices[0]) * _indexBufferComponent.Count;
	uint32 indexSize = sizeof(_indices[0]);

	BufferComponent stagingBuffer;
	m_buffer->Create(
		stagingBuffer,
		indexSize,
		_indexBufferComponent.Count,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	void* data;
	m_buffer->Map(stagingBuffer, &data);
	m_buffer->WriteToBuffer(data, (void*)_indices.data(), static_cast<std::size_t>(bufferSize));
	m_buffer->Unmap(stagingBuffer);

	m_buffer->Create(
		_indexBufferComponent,
		indexSize,
		_indexBufferComponent.Count,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	m_buffer->Copy(stagingBuffer, _indexBufferComponent, bufferSize);

	m_buffer->Destroy(stagingBuffer);
}

VESPERENGINE_NAMESPACE_END
