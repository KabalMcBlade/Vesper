#include "pch.h"
#include "model_system.h"

#include "Core/memory_copy.h"
#include "Backend/device.h"

#include "ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

ModelSystem::ModelSystem(Device& _device)
	: m_device{ _device }
{
}

void ModelSystem::LoadModel(ecs::Entity _entity, std::shared_ptr<ModelData> _data) const
{
	// depending by the data, add the component
	if (_data->IsStatic)
	{
		ecs::ComponentManager::AddComponent<StaticComponent>(_entity);
	}

	// If has both vertex buffers and index buffer, we simple use the compound component
	if (_data->Vertices.size() > 0 && _data->Indices.size() > 0)
	{
		ecs::ComponentManager::AddComponent<VertexAndIndexBufferComponent>(_entity);

		VertexAndIndexBufferComponent& vertexAndIndexBuffer = ecs::ComponentManager::GetComponent<VertexAndIndexBufferComponent>(_entity);

		if (_data->IsStatic)
		{
			CreateVertexBuffersWithStagingBuffer(vertexAndIndexBuffer.VertexBuffer, _data->Vertices);
			CreateIndexBufferWithStagingBuffer(vertexAndIndexBuffer.IndexBuffer, _data->Indices);
		}
		else
		{
			CreateVertexBuffers(vertexAndIndexBuffer.VertexBuffer, _data->Vertices);
			CreateIndexBuffer(vertexAndIndexBuffer.IndexBuffer, _data->Indices);
		}
	}
	// otherwise just add the vertex buffer, which should be mandatory at least
	else if (_data->Vertices.size() > 0)
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

	// material later
}

void ModelSystem::UnloadModel(ecs::Entity _entity) const
{
	if (ecs::ComponentManager::HasComponents<VertexAndIndexBufferComponent>(_entity))
	{
		VertexAndIndexBufferComponent& vertexAndIndexBuffer = ecs::ComponentManager::GetComponent<VertexAndIndexBufferComponent>(_entity);
		vmaDestroyBuffer(m_device.GetAllocator(), vertexAndIndexBuffer.VertexBuffer.Buffer, vertexAndIndexBuffer.VertexBuffer.BufferMemory);
		vmaDestroyBuffer(m_device.GetAllocator(), vertexAndIndexBuffer.IndexBuffer.Buffer, vertexAndIndexBuffer.IndexBuffer.BufferMemory);

		ecs::ComponentManager::RemoveComponent<VertexAndIndexBufferComponent>(_entity);
	}

	if (ecs::ComponentManager::HasComponents<VertexBufferComponent>(_entity))
	{
		VertexBufferComponent& vertexBuffer = ecs::ComponentManager::GetComponent<VertexBufferComponent>(_entity);
		vmaDestroyBuffer(m_device.GetAllocator(), vertexBuffer.Buffer, vertexBuffer.BufferMemory);

		ecs::ComponentManager::RemoveComponent<VertexBufferComponent>(_entity);
	}

	// NO INDEX BUFFER ONLY!
// 	if (ecs::ComponentManager::HasComponents<IndexBufferComponent>(_entity))
// 	{
// 		IndexBufferComponent& indexBuffer = ecs::ComponentManager::GetComponent<IndexBufferComponent>(_entity);
// 		vmaDestroyBuffer(m_device.GetAllocator(), indexBuffer.Buffer, indexBuffer.BufferMemory);
// 
// 		ecs::ComponentManager::RemoveComponent<IndexBufferComponent>(_entity);
// 	}

	if (ecs::ComponentManager::HasComponents<StaticComponent>(_entity))
	{
		ecs::ComponentManager::RemoveComponent<StaticComponent>(_entity);
	}
	// material later
}

void ModelSystem::LoadModels(std::shared_ptr<ModelData> _data) const
{
	for (auto iterator : ecs::IterateEntitiesWithAll<VertexBufferComponent>())
	{
		LoadModel(iterator, _data);
	}
}

void ModelSystem::UnloadModels() const
{
	for (auto iterator : ecs::IterateEntitiesWithAll<VertexAndIndexBufferComponent>())
	{
		UnloadModel(iterator);
	}

	for (auto iterator : ecs::IterateEntitiesWithAll<VertexBufferComponent>())
	{
		UnloadModel(iterator);
	}
}

void ModelSystem::CreateVertexBuffers(VertexBufferComponent& _vertexBufferComponent, const std::vector<Vertex>& _vertices) const
{
	_vertexBufferComponent.Count = static_cast<uint32>(_vertices.size());

	assertMsgReturnVoid(_vertexBufferComponent.Count >= 3, "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertexBufferComponent.Count;

	m_device.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		_vertexBufferComponent.Buffer,
		_vertexBufferComponent.BufferMemory
	);

	void* data;
	vmaMapMemory(m_device.GetAllocator(), _vertexBufferComponent.BufferMemory, &data);
	MemCpy(data, _vertices.data(), static_cast<std::size_t>(bufferSize));
	vmaUnmapMemory(m_device.GetAllocator(), _vertexBufferComponent.BufferMemory);
}

void ModelSystem::CreateIndexBuffer(IndexBufferComponent& _indexBufferComponent, const std::vector<uint32>& _indices) const
{
	_indexBufferComponent.Count = static_cast<uint32>(_indices.size());

	VkDeviceSize bufferSize = sizeof(_indices[0]) * _indexBufferComponent.Count;

	m_device.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		_indexBufferComponent.Buffer,
		_indexBufferComponent.BufferMemory
	);

	void* data;
	vmaMapMemory(m_device.GetAllocator(), _indexBufferComponent.BufferMemory, &data);
	MemCpy(data, _indices.data(), static_cast<std::size_t>(bufferSize));
	vmaUnmapMemory(m_device.GetAllocator(), _indexBufferComponent.BufferMemory);
}


void ModelSystem::CreateVertexBuffersWithStagingBuffer(VertexBufferComponent& _vertexBufferComponent, const std::vector<Vertex>& _vertices) const
{
	_vertexBufferComponent.Count = static_cast<uint32>(_vertices.size());

	assertMsgReturnVoid(_vertexBufferComponent.Count >= 3, "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertexBufferComponent.Count;

	VkBuffer stagingBuffer;
	VmaAllocation staginBufferMemory;
	m_device.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		stagingBuffer,
		staginBufferMemory
	);

	void* data;
	vmaMapMemory(m_device.GetAllocator(), staginBufferMemory, &data);
	MemCpy(data, _vertices.data(), static_cast<std::size_t>(bufferSize));
	vmaUnmapMemory(m_device.GetAllocator(), staginBufferMemory);

	m_device.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		_vertexBufferComponent.Buffer,
		_vertexBufferComponent.BufferMemory
	);


	m_device.CopyBuffer(stagingBuffer, _vertexBufferComponent.Buffer, bufferSize);

	vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer, staginBufferMemory);
}

void ModelSystem::CreateIndexBufferWithStagingBuffer(IndexBufferComponent& _indexBufferComponent, const std::vector<uint32>& _indices) const
{
	_indexBufferComponent.Count = static_cast<uint32>(_indices.size());

	VkDeviceSize bufferSize = sizeof(_indices[0]) * _indexBufferComponent.Count;

	VkBuffer stagingBuffer;
	VmaAllocation staginBufferMemory;
	m_device.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		stagingBuffer,
		staginBufferMemory
	);

	void* data;
	vmaMapMemory(m_device.GetAllocator(), staginBufferMemory, &data);
	MemCpy(data, _indices.data(), static_cast<std::size_t>(bufferSize));
	vmaUnmapMemory(m_device.GetAllocator(), staginBufferMemory);
	m_device.CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		_indexBufferComponent.Buffer,
		_indexBufferComponent.BufferMemory
	);

	m_device.CopyBuffer(stagingBuffer, _indexBufferComponent.Buffer, bufferSize);

	vmaDestroyBuffer(m_device.GetAllocator(), stagingBuffer, staginBufferMemory);
}

VESPERENGINE_NAMESPACE_END
