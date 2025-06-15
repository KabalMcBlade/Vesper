// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Systems\model_system.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Systems/model_system.h"
#include "Systems/material_system.h"

#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/model_data.h"

#include "Components/pipeline_components.h"
#include "Components/object_components.h"

#include "App/vesper_app.h"

#include "ECS/ECS/ecs.h"


VESPERENGINE_NAMESPACE_BEGIN

ModelSystem::ModelSystem(VesperApp& _app, Device& _device, MaterialSystem& _materialSystem)
	: m_app(_app)
	, m_device{ _device }
	, m_materialSystem{ _materialSystem }
{
	m_buffer = std::make_unique<Buffer>(m_device);
}

void ModelSystem::LoadModel(ecs::Entity _entity, std::shared_ptr<ModelData> _data) const
{
	switch (_data->Material->Type)
	{
	case MaterialType::Phong:
		{
			m_app.GetComponentManager().AddComponent<PhongMaterialComponent>(_entity);
			PhongMaterialComponent& phongMaterialComponent = m_app.GetComponentManager().GetComponent<PhongMaterialComponent>(_entity);
			
			phongMaterialComponent.Index = _data->Material->Index;

			phongMaterialComponent.AmbientImageInfo.sampler = _data->Material->Textures[0]->Sampler;
			phongMaterialComponent.AmbientImageInfo.imageView = _data->Material->Textures[0]->ImageView;
			phongMaterialComponent.AmbientImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			phongMaterialComponent.DiffuseImageInfo.sampler = _data->Material->Textures[1]->Sampler;
			phongMaterialComponent.DiffuseImageInfo.imageView = _data->Material->Textures[1]->ImageView;
			phongMaterialComponent.DiffuseImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			phongMaterialComponent.SpecularImageInfo.sampler = _data->Material->Textures[2]->Sampler;
			phongMaterialComponent.SpecularImageInfo.imageView = _data->Material->Textures[2]->ImageView;
			phongMaterialComponent.SpecularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            phongMaterialComponent.NormalImageInfo.sampler = _data->Material->Textures[3]->Sampler;
            phongMaterialComponent.NormalImageInfo.imageView = _data->Material->Textures[3]->ImageView;
            phongMaterialComponent.NormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            phongMaterialComponent.AlphaImageInfo.sampler = _data->Material->Textures[4]->Sampler;
            phongMaterialComponent.AlphaImageInfo.imageView = _data->Material->Textures[4]->ImageView;
            phongMaterialComponent.AlphaImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            phongMaterialComponent.UniformBufferInfo = m_buffer->GetDescriptorInfo(_data->Material->UniformBuffer);
		}
		break;
	case MaterialType::PBR:
		{
			m_app.GetComponentManager().AddComponent<PBRMaterialComponent>(_entity);
			PBRMaterialComponent& pbrMaterialComponent = m_app.GetComponentManager().GetComponent<PBRMaterialComponent>(_entity);

			pbrMaterialComponent.Index = _data->Material->Index;

			pbrMaterialComponent.RoughnessImageInfo.sampler = _data->Material->Textures[0]->Sampler;
			pbrMaterialComponent.RoughnessImageInfo.imageView = _data->Material->Textures[0]->ImageView;
			pbrMaterialComponent.RoughnessImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.MetallicImageInfo.sampler = _data->Material->Textures[1]->Sampler;
			pbrMaterialComponent.MetallicImageInfo.imageView = _data->Material->Textures[1]->ImageView;
			pbrMaterialComponent.MetallicImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.SheenImageInfo.sampler = _data->Material->Textures[2]->Sampler;
			pbrMaterialComponent.SheenImageInfo.imageView = _data->Material->Textures[2]->ImageView;
			pbrMaterialComponent.SheenImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.EmissiveImageInfo.sampler = _data->Material->Textures[3]->Sampler;
			pbrMaterialComponent.EmissiveImageInfo.imageView = _data->Material->Textures[3]->ImageView;
			pbrMaterialComponent.EmissiveImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.NormalImageInfo.sampler = _data->Material->Textures[4]->Sampler;
			pbrMaterialComponent.NormalImageInfo.imageView = _data->Material->Textures[4]->ImageView;
			pbrMaterialComponent.NormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.UniformBufferInfo = m_buffer->GetDescriptorInfo(_data->Material->UniformBuffer);
		}
		break;
	default:
		throw std::runtime_error("Unsupported material type");
		break;
	}

	if (_data->Material->IsTransparent)
	{
		m_app.GetComponentManager().AddComponent<PipelineTransparentComponent>(_entity);
	}
	else
	{
		m_app.GetComponentManager().AddComponent<PipelineOpaqueComponent>(_entity);
	}

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
}

void ModelSystem::LoadSkyboxModel(ecs::Entity _entity, std::shared_ptr<ModelData> _modelData, std::shared_ptr<TextureData> _textureData) const
{
	m_app.GetComponentManager().AddComponent<PipelineSkyboxComponent>(_entity);

	m_app.GetComponentManager().AddComponent<SkyboxMaterialComponent>(_entity);
	SkyboxMaterialComponent& sb = m_app.GetComponentManager().GetComponent<SkyboxMaterialComponent>(_entity);
	sb.ImageInfo.sampler = _textureData->Sampler;
	sb.ImageInfo.imageView = _textureData->ImageView;
	sb.ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// depending by the data, add the component
	if (_modelData->IsStatic)
	{
		m_app.GetComponentManager().AddComponent<StaticComponent>(_entity);
	}

	if (_modelData->Vertices.size() > 0)
	{
		m_app.GetComponentManager().AddComponent<VertexBufferComponent>(_entity);

		VertexBufferComponent& vertexBuffer = m_app.GetComponentManager().GetComponent<VertexBufferComponent>(_entity);

		if (m_app.GetComponentManager().HasComponents<StaticComponent>(_entity))
		{
			vertexBuffer = CreateVertexBuffersWithStagingBuffer(_modelData->Vertices);
		}
		else
		{
			vertexBuffer = CreateVertexBuffers(_modelData->Vertices);
		}
	}
	else
	{
		m_app.GetComponentManager().AddComponent<NotVertexBufferComponent>(_entity);
	}

	if (_modelData->Indices.size() > 0)
	{
		m_app.GetComponentManager().AddComponent<IndexBufferComponent>(_entity);

		IndexBufferComponent& indexBuffer = m_app.GetComponentManager().GetComponent<IndexBufferComponent>(_entity);

		if (_modelData->IsStatic)
		{
			indexBuffer = CreateIndexBufferWithStagingBuffer(_modelData->Indices);
		}
		else
		{
			indexBuffer = CreateIndexBuffer(_modelData->Indices);
		}
	}
	else
	{
		m_app.GetComponentManager().AddComponent<NotIndexBufferComponent>(_entity);
	}
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

	if (m_app.GetComponentManager().HasComponents<PhongMaterialComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<PhongMaterialComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<PBRMaterialComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<PBRMaterialComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<NoMaterialComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<NoMaterialComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<PipelineTransparentComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<PipelineTransparentComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<PipelineOpaqueComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<PipelineOpaqueComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<PipelineSkyboxComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<PipelineSkyboxComponent>(_entity);
	}

	if (m_app.GetComponentManager().HasComponents<SkyboxMaterialComponent>(_entity))
	{
		m_app.GetComponentManager().RemoveComponent<SkyboxMaterialComponent>(_entity);
	}
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

	m_buffer->Map(vertexBufferComponent);
	m_buffer->WriteToBuffer(vertexBufferComponent.MappedMemory, (void*)_vertices.data(), static_cast<std::size_t>(bufferSize));
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

	m_buffer->Map(indexBufferComponent);
	m_buffer->WriteToBuffer(indexBufferComponent.MappedMemory, (void*)_indices.data(), static_cast<std::size_t>(bufferSize));
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

	m_buffer->Map(stagingBuffer);
	m_buffer->WriteToBuffer(stagingBuffer.MappedMemory, (void*)_vertices.data(), static_cast<std::size_t>(bufferSize));
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

	m_buffer->Map(stagingBuffer);
	m_buffer->WriteToBuffer(stagingBuffer.MappedMemory, (void*)_indices.data(), static_cast<std::size_t>(bufferSize));
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
