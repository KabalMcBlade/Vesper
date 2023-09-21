#include "pch.h"

#include "Core/memory_copy.h"
#include "Geometry/model.h"
#include "Backend/device.h"


VESPERENGINE_NAMESPACE_BEGIN


std::vector<VkVertexInputBindingDescription> Model::Vertex::GetBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;	// vec4
	attributeDescriptions[0].offset = offsetof(Vertex, Position);

	attributeDescriptions[1].binding = 0;	// binding still 0, because the Position and Color are interleaving in the same binding
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;	// vec4
	attributeDescriptions[1].offset = offsetof(Vertex, Color);

	return attributeDescriptions;
}

Model::Model(Device& _device, const std::vector<Vertex> _vertices)
	: m_device{_device}
{
	CreateVertexBuffers(_vertices);
}

Model::~Model()
{
	vmaDestroyBuffer(m_device.GetAllocator(), m_vertexBuffer, m_vertexBufferMemory);
}

void Model::Bind(VkCommandBuffer _commandBuffer)
{
	VkBuffer buffers[] = { m_vertexBuffer };
	VkDeviceSize offsets[] = {0};

	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, buffers, offsets);
}

void Model::Draw(VkCommandBuffer _commandBuffer)
{
	vkCmdDraw(_commandBuffer, m_vertexCount, 1, 0, 0);
}

void Model::CreateVertexBuffers(const std::vector<Vertex> _vertices)
{
	m_vertexCount = static_cast<uint32>(_vertices.size());

	assertMsgReturnVoid(m_vertexCount >= 3, "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(_vertices[0]) * m_vertexCount;

	m_device.CreateBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
		m_vertexBuffer,
		m_vertexBufferMemory
	);

	void *data;
	vmaMapMemory(m_device.GetAllocator(), m_vertexBufferMemory, &data);

	MemCpy(data, _vertices.data(), static_cast<std::size_t>(bufferSize));

	vmaUnmapMemory(m_device.GetAllocator(), m_vertexBufferMemory);
}


VESPERENGINE_NAMESPACE_END
