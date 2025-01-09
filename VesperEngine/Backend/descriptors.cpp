// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\descriptors.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Backend/descriptors.h"

#include "Utility/logger.h"

#include <cassert>


VESPERENGINE_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(
	uint32 _binding,
	VkDescriptorType _descriptorType,
	VkShaderStageFlags _stageFlags,
	uint32 _count) 
{
	assert(m_bindings.count(_binding) == 0 && "Binding already in use");

	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = _binding;
	layoutBinding.descriptorType = _descriptorType;
	layoutBinding.descriptorCount = _count;
	layoutBinding.stageFlags = _stageFlags;
	m_bindings[_binding] = layoutBinding;
	return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const 
{
	return std::make_unique<DescriptorSetLayout>(m_device, m_bindings);
}

DescriptorSetLayout::DescriptorSetLayout(
	Device& _device, 
	std::unordered_map<uint32, VkDescriptorSetLayoutBinding> _bindings)
	: m_device{ _device }
	, m_bindings{ _bindings }
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
	for (auto kv : m_bindings) 
	{
		setLayoutBindings.push_back(kv.second);
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32>(setLayoutBindings.size());
	descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

	if (vkCreateDescriptorSetLayout(m_device.GetDevice(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(m_device.GetDevice(), m_descriptorSetLayout, nullptr);
}

//////////////////////////////////////////////////////////////////////////

DescriptorPool::Builder& DescriptorPool::Builder::AddPoolSize(VkDescriptorType _descriptorType, uint32 _count)
{
	m_poolSizes.push_back({ _descriptorType, _count });
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags _flags) 
{
	m_poolFlags = _flags;
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetMaxSets(uint32 _count) 
{
	m_maxSets = _count;
	return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build() const 
{
	return std::make_unique<DescriptorPool>(m_device, m_maxSets, m_poolFlags, m_poolSizes);
}

//////////////////////////////////////////////////////////////////////////

DescriptorPool::DescriptorPool(
	Device& m_device,
	uint32 _maxSets,
	VkDescriptorPoolCreateFlags _poolFlags,
	const std::vector<VkDescriptorPoolSize>& _poolSizes)
	: m_device{ m_device }
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32>(_poolSizes.size());
	descriptorPoolInfo.pPoolSizes = _poolSizes.data();
	descriptorPoolInfo.maxSets = _maxSets;
	descriptorPoolInfo.flags = _poolFlags;

	if (vkCreateDescriptorPool(m_device.GetDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(m_device.GetDevice(), m_descriptorPool, nullptr);
}

bool DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout _descriptorSetLayout, VkDescriptorSet& _descriptor) const
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.pSetLayouts = &_descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	// Might want to create a "DescriptorPoolManager" class that handles this case, and builds a new pool whenever an old pool fills up.
	VkResult result = vkAllocateDescriptorSets(m_device.GetDevice(), &allocInfo, &_descriptor);
	if (result != VK_SUCCESS)
	{
		switch (result) 
		{
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			LOG(Logger::ERROR, "Error: Out of host memory while allocating descriptor sets.");
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			LOG(Logger::ERROR, "Error: Out of device memory while allocating descriptor sets.");
			break;
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			LOG(Logger::ERROR, "Error: Descriptor pool ran out of memory.");
			break;
		case VK_ERROR_FRAGMENTATION:
			LOG(Logger::ERROR, "Error: Fragmentation of pool memory while allocating descriptor sets.");
			break;
		default:
			LOG(Logger::ERROR, "Error: vkAllocateDescriptorSets failed with unknown error: ", result);
			break;
		}

		return false;
	}
	return true;
}

void DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet>& _descriptors) const
{
	vkFreeDescriptorSets(
		m_device.GetDevice(),
		m_descriptorPool,
		static_cast<uint32>(_descriptors.size()),
		_descriptors.data());
}

void DescriptorPool::ResetPool()
{
	vkResetDescriptorPool(m_device.GetDevice(), m_descriptorPool, 0);
}

//////////////////////////////////////////////////////////////////////////

DescriptorWriter::DescriptorWriter(DescriptorSetLayout& _setLayout, DescriptorPool& _pool)
	: m_setLayout{ _setLayout }
	, m_pool{ _pool } {}

DescriptorWriter& DescriptorWriter::WriteBuffer(uint32 _binding, VkDescriptorBufferInfo* _bufferInfo) 
{
	assert(m_setLayout.m_bindings.count(_binding) == 1 && "Layout does not contain specified _binding");

	auto& bindingDescription = m_setLayout.m_bindings[_binding];

	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but _binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = _binding;
	write.pBufferInfo = _bufferInfo;
	write.descriptorCount = 1;

	m_writes.push_back(write);
	return *this;
}

DescriptorWriter& DescriptorWriter::WriteImage(uint32 _binding, VkDescriptorImageInfo* _imageInfo)
{
	assert(m_setLayout.m_bindings.count(_binding) == 1 && "Layout does not contain specified _binding");

	auto& bindingDescription = m_setLayout.m_bindings[_binding];

	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but _binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = _binding;
	write.pImageInfo = _imageInfo;
	write.descriptorCount = 1;

	m_writes.push_back(write);
	return *this;
}

bool DescriptorWriter::Build(VkDescriptorSet& _set) 
{
	bool success = m_pool.AllocateDescriptorSet(m_setLayout.GetDescriptorSetLayout(), _set);
	if (!success) 
	{
		LOG(Logger::ERROR, "Cannot build the descriptor set!");

		return false;
	}
	Overwrite(_set);
	return true;
}

void DescriptorWriter::Overwrite(VkDescriptorSet& _set)
{
	for (auto& write : m_writes)
	{
		write.dstSet = _set;
	}
	vkUpdateDescriptorSets(m_pool.m_device.GetDevice(), static_cast<uint32>(m_writes.size()), m_writes.data(), 0, nullptr);
}

VESPERENGINE_NAMESPACE_END
