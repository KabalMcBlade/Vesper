#include "pch.h"
#include "model_system.h"

#include "Core/memory_copy.h"
#include "Backend/device.h"

#include "App/vesper_app.h"


VESPERENGINE_NAMESPACE_BEGIN

ModelSystem::ModelSystem(VesperApp& _app, Device& _device)
	: m_app(_app)
	, m_device{ _device }
{
	m_buffer = std::make_unique<Buffer>(m_device);
}

void ModelSystem::LoadModel(ecs::Entity _entity, std::shared_ptr<ModelData> _data) const
{
	// depending by the data, add the component
	if (_data->IsStatic)
	{
		m_app.GetComponentManager().AddComponent<StaticComponent>(_entity);
	}

	if (_data->Vertices.size() > 0)
	{
		m_app.GetComponentManager().AddComponent<VertexBufferComponent>(_entity);

		VertexBufferComponent& vertexBuffer = m_app.GetComponentManager().GetComponent<VertexBufferComponent>(_entity);

		if (m_app.GetComponentManager().HasComponents<StaticComponent>(_entity))
		{
			vertexBuffer = CreateVertexBuffersWithStagingBuffer(_data->Vertices);
		}
		else
		{
			vertexBuffer = CreateVertexBuffers(_data->Vertices);
		}
	}
	else
	{
		m_app.GetComponentManager().AddComponent<NotVertexBufferComponent>(_entity);
	}

	if (_data->Indices.size() > 0)
	{
		m_app.GetComponentManager().AddComponent<IndexBufferComponent>(_entity);

		IndexBufferComponent& indexBuffer = m_app.GetComponentManager().GetComponent<IndexBufferComponent>(_entity);

		if (_data->IsStatic)
		{
			indexBuffer = CreateIndexBufferWithStagingBuffer(_data->Indices);
		}
		else
		{
			indexBuffer = CreateIndexBuffer(_data->Indices);
		}
	}
	else
	{
		m_app.GetComponentManager().AddComponent<NotIndexBufferComponent>(_entity);
	}

	// material later
}

void ModelSystem::UnloadModel(ecs::Entity _entity) const
{
	if (m_app.GetComponentManager().HasComponents<VertexBufferComponent>(_entity))
	{
		VertexBufferComponent& vertexBuffer = m_app.GetComponentManager().GetComponent<VertexBufferComponent>(_entity);

		m_buffer->Destroy(vertexBuffer);

		m_app.GetComponentManager().RemoveComponent<VertexBufferComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<IndexBufferComponent>(_entity))
	{
		IndexBufferComponent& indexBuffer = m_app.GetComponentManager().GetComponent<IndexBufferComponent>(_entity);

		m_buffer->Destroy(indexBuffer);

		m_app.GetComponentManager().RemoveComponent<IndexBufferComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<NotVertexBufferComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<NotVertexBufferComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<NotIndexBufferComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<NotIndexBufferComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<StaticComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<StaticComponent>(_entity);
	}

	// material later
}

void ModelSystem::UnloadModels() const
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto iterator : ecs::IterateEntitiesWithAny<VertexBufferComponent, IndexBufferComponent>(entityManager, componentManager))
	{
		UnloadModel(iterator);
	}
}

VertexBufferComponent ModelSystem::CreateVertexBuffers(const std::vector<Vertex>& _vertices) const
{
	const uint32 vertexCount = static_cast<uint32>(_vertices.size());

	assertMsgReturnValue(vertexCount >= 3, "Vertex count must be at least 3", VertexBufferComponent());

	const VkDeviceSize bufferSize = sizeof(_vertices[0]) * vertexCount;
	const uint32 vertexSize = sizeof(_vertices[0]);

	VertexBufferComponent vertexBufferComponent = m_buffer->Create<VertexBufferComponent>(
		vertexSize,
		vertexCount,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	void* data;
	m_buffer->Map(vertexBufferComponent, &data);
	m_buffer->WriteToBuffer(data, (void*)_vertices.data(), static_cast<std::size_t>(bufferSize));
	m_buffer->Unmap(vertexBufferComponent);

	return vertexBufferComponent;
}

IndexBufferComponent ModelSystem::CreateIndexBuffer(const std::vector<uint32>& _indices) const
{
	const uint32 indexCount = static_cast<uint32>(_indices.size());

	const VkDeviceSize bufferSize = sizeof(_indices[0]) * indexCount;
	const uint32 indexSize = sizeof(_indices[0]);

	IndexBufferComponent indexBufferComponent = m_buffer->Create<IndexBufferComponent>(
		indexSize,
		indexCount,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	void* data;
	m_buffer->Map(indexBufferComponent, &data);
	m_buffer->WriteToBuffer(data, (void*)_indices.data(), static_cast<std::size_t>(bufferSize));
	m_buffer->Unmap(indexBufferComponent);

	return indexBufferComponent;
}


VertexBufferComponent ModelSystem::CreateVertexBuffersWithStagingBuffer(const std::vector<Vertex>& _vertices) const
{
	const uint32 vertexCount = static_cast<uint32>(_vertices.size());

	assertMsgReturnValue(vertexCount >= 3, "Vertex count must be at least 3", VertexBufferComponent());

	const VkDeviceSize bufferSize = sizeof(_vertices[0]) * vertexCount;
	const uint32 vertexSize = sizeof(_vertices[0]);

	BufferComponent stagingBuffer = m_buffer->Create<BufferComponent>(
		vertexSize,
		vertexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	void* data;
	m_buffer->Map(stagingBuffer, &data);
	m_buffer->WriteToBuffer(data, (void*)_vertices.data(), static_cast<std::size_t>(bufferSize));
	m_buffer->Unmap(stagingBuffer);
	
	VertexBufferComponent vertexBufferComponent = m_buffer->Create<VertexBufferComponent>(
		vertexSize,
		vertexCount,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	m_buffer->Copy(stagingBuffer, vertexBufferComponent, bufferSize);

	m_buffer->Destroy(stagingBuffer);

	return vertexBufferComponent;
}

IndexBufferComponent ModelSystem::CreateIndexBufferWithStagingBuffer(const std::vector<uint32>& _indices) const
{
	const uint32 indexCount = static_cast<uint32>(_indices.size());

	VkDeviceSize bufferSize = sizeof(_indices[0]) * indexCount;
	uint32 indexSize = sizeof(_indices[0]);

	BufferComponent stagingBuffer = m_buffer->Create<BufferComponent>(
		indexSize,
		indexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	void* data;
	m_buffer->Map(stagingBuffer, &data);
	m_buffer->WriteToBuffer(data, (void*)_indices.data(), static_cast<std::size_t>(bufferSize));
	m_buffer->Unmap(stagingBuffer);

	IndexBufferComponent indexBufferComponent = m_buffer->Create<IndexBufferComponent>(
		indexSize,
		indexCount,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
	);

	m_buffer->Copy(stagingBuffer, indexBufferComponent, bufferSize);

	m_buffer->Destroy(stagingBuffer);

	return indexBufferComponent;
}

VESPERENGINE_NAMESPACE_END
