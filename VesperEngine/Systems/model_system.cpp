#include "model_system.h"

#include "Core/memory_copy.h"

#include "Backend/device.h"

#include "App/vesper_app.h"

#include "Utility/logger.h"


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
	// create real materials and store in the material system, once the model is getting loaded, so for each model
	// create a new material, if the same material parameters don't exist, otherwise reuse a material with the same data
	int32 materialIndex = -1;
	if (_data->Material)
	{
		materialIndex = m_materialSystem.CreateMaterial(*_data->Material);

		if (materialIndex == -1)
		{
			LOG(Logger::ERROR, "Material ", _data->Material->Name, " is present and valid, but couldn't create or retrieve it");
		}
	}
			
	// add material component and bind it
	switch (_data->Material->Type)
	{
	case MaterialType::Phong:
		{
			const auto material = m_materialSystem.GetMaterial(materialIndex);
			auto materialPhong = std::dynamic_pointer_cast<MaterialPhong>(material);

			m_app.GetComponentManager().AddComponent<PhongMaterialComponent>(_entity);
			PhongMaterialComponent& phongMaterialComponent = m_app.GetComponentManager().GetComponent<PhongMaterialComponent>(_entity);
			
			phongMaterialComponent.Index = materialIndex;

			phongMaterialComponent.DiffuseImageInfo.sampler = materialPhong->DiffuseTexture.Sampler;
			phongMaterialComponent.DiffuseImageInfo.imageView = materialPhong->DiffuseTexture.ImageView;
			phongMaterialComponent.DiffuseImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			phongMaterialComponent.SpecularImageInfo.sampler = materialPhong->SpecularTexture.Sampler;
			phongMaterialComponent.SpecularImageInfo.imageView = materialPhong->SpecularTexture.ImageView;
			phongMaterialComponent.SpecularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			phongMaterialComponent.AmbientImageInfo.sampler = materialPhong->AmbientTexture.Sampler;
			phongMaterialComponent.AmbientImageInfo.imageView = materialPhong->AmbientTexture.ImageView;
			phongMaterialComponent.AmbientImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			phongMaterialComponent.NormalImageInfo.sampler = materialPhong->NormalTexture.Sampler;
			phongMaterialComponent.NormalImageInfo.imageView = materialPhong->NormalTexture.ImageView;
			phongMaterialComponent.NormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

 			phongMaterialComponent.UniformBufferInfo.buffer = materialPhong->UniformBuffer.Buffer;
 			phongMaterialComponent.UniformBufferInfo.offset = 0;
 			phongMaterialComponent.UniformBufferInfo.range = materialPhong->UniformBuffer.AlignedSize;
		}
		break;
	case MaterialType::PBR:
		{
			const auto material = m_materialSystem.GetMaterial(materialIndex);
			auto materialPBR = std::dynamic_pointer_cast<MaterialPBR>(material);

			m_app.GetComponentManager().AddComponent<PBRMaterialComponent>(_entity);
			PBRMaterialComponent& pbrMaterialComponent = m_app.GetComponentManager().GetComponent<PBRMaterialComponent>(_entity);

			pbrMaterialComponent.Index = materialIndex;

			pbrMaterialComponent.RoughnessImageInfo.sampler = materialPBR->RoughnessTexture.Sampler;
			pbrMaterialComponent.RoughnessImageInfo.imageView = materialPBR->RoughnessTexture.ImageView;
			pbrMaterialComponent.RoughnessImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.MetallicImageInfo.sampler = materialPBR->MetallicTexture.Sampler;
			pbrMaterialComponent.MetallicImageInfo.imageView = materialPBR->MetallicTexture.ImageView;
			pbrMaterialComponent.MetallicImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.SheenImageInfo.sampler = materialPBR->SheenTexture.Sampler;
			pbrMaterialComponent.SheenImageInfo.imageView = materialPBR->SheenTexture.ImageView;
			pbrMaterialComponent.SheenImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.EmissiveImageInfo.sampler = materialPBR->EmissiveTexture.Sampler;
			pbrMaterialComponent.EmissiveImageInfo.imageView = materialPBR->EmissiveTexture.ImageView;
			pbrMaterialComponent.EmissiveImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.NormalImageInfo.sampler = materialPBR->NormalMapTexture.Sampler;
			pbrMaterialComponent.NormalImageInfo.imageView = materialPBR->NormalMapTexture.ImageView;
			pbrMaterialComponent.NormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			pbrMaterialComponent.UniformBufferInfo.buffer = materialPBR->UniformBuffer.Buffer;
			pbrMaterialComponent.UniformBufferInfo.offset = 0;
			pbrMaterialComponent.UniformBufferInfo.range = materialPBR->UniformBuffer.AlignedSize;
		}
		break;
	default:
		throw std::runtime_error("Unsupported material type");
		break;
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
