#pragma once

#include "Core/core_defines.h"

#include "Backend/device.h"

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vma/vk_mem_alloc.h>

#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_DLL Model
{
public:
	struct Vertex
	{
		glm::vec4 Position;
		glm::vec4 Color;
		
		static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	};

public:
	Model(Device& _device, const std::vector<Vertex> _vertices);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

public:
	void Bind(VkCommandBuffer _commandBuffer);
	void Draw(VkCommandBuffer _commandBuffer);

private:
	void CreateVertexBuffers(const std::vector<Vertex> _vertices);

private:
	Device& m_device;
	VkBuffer m_vertexBuffer;
	VmaAllocation m_vertexBufferMemory;
	uint32 m_vertexCount;	
};

VESPERENGINE_NAMESPACE_END