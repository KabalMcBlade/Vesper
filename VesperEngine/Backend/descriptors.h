// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\descriptors.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "vulkan/vulkan.h"

#include <unordered_map>
#include <memory>
#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class Device;

//////////////////////////////////////////////////////////////////////////
class VESPERENGINE_API DescriptorSetLayout
{
public:
	class VESPERENGINE_API Builder
	{
	public:
		Builder(Device& _device) : m_device{ _device } {}

		Builder& AddBinding(
			uint32 _binding,
			VkDescriptorType _descriptorType,
			VkShaderStageFlags _stageFlags,
			uint32 _maxCount = 1,
			uint32 _count = 1);
		Builder& SetFlags(VkDescriptorSetLayoutCreateFlags _flags);
		std::unique_ptr<DescriptorSetLayout> Build() const;

	private:
		Device& m_device;
		VkDescriptorSetLayoutCreateFlags m_flags = 0;
		std::unordered_map<uint32, VkDescriptorSetLayoutBinding> m_bindings{};
		std::unordered_map<uint32, uint32> m_sizePerBinding{};	// used for bindless binding
	};

	DescriptorSetLayout(Device& _device,
		VkDescriptorSetLayoutCreateFlags _flags,
		std::unordered_map<uint32, VkDescriptorSetLayoutBinding> _bindings,
		std::unordered_map<uint32, uint32> _sizePerBinding);
	~DescriptorSetLayout();
	DescriptorSetLayout(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

	VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }
	//const std::unordered_map<uint32, VkDescriptorSetLayoutBinding>& GetBindings() const { return m_bindings; }
	const std::unordered_map<uint32, uint32>& GetSizePerBinding() const { return m_sizePerBinding; }
	VkDescriptorSetLayoutCreateFlags GetLayoutFlags() const { return m_layoutFlags; }

private:
	Device& m_device;
	VkDescriptorSetLayout m_descriptorSetLayout;
	std::unordered_map<uint32, VkDescriptorSetLayoutBinding> m_bindings{};
	std::unordered_map<uint32, uint32> m_sizePerBinding{};	// used for bindless binding
	VkDescriptorSetLayoutCreateFlags m_layoutFlags{0};		// used for bindless binding

	friend class DescriptorWriter;
};

//////////////////////////////////////////////////////////////////////////
class VESPERENGINE_API DescriptorPool
{
public:
	class VESPERENGINE_API Builder
	{
	public:
		Builder(Device& _device) : m_device{ _device } {}

		Builder& AddPoolSize(VkDescriptorType _descriptorType, uint32 _count);
		Builder& SetPoolFlags(VkDescriptorPoolCreateFlags _flags);
		Builder& SetMaxSets(uint32 _count);
		std::unique_ptr<DescriptorPool> Build() const;

	private:
		Device& m_device;
		std::vector<VkDescriptorPoolSize> m_poolSizes{};
		uint32 m_maxSets = 1000;
		VkDescriptorPoolCreateFlags m_poolFlags = 0;
	};

	DescriptorPool(
		Device& _device,
		uint32 _totalCountDescriptorSets,
		VkDescriptorPoolCreateFlags _poolFlags,
		const std::vector<VkDescriptorPoolSize>& _poolSizes);
	~DescriptorPool();
	DescriptorPool(const DescriptorPool&) = delete;
	DescriptorPool& operator=(const DescriptorPool&) = delete;

	bool AllocateDescriptorSet(const DescriptorSetLayout& _descriptorSetLayout, VkDescriptorSet& _descriptor) const;
	void FreeDescriptors(std::vector<VkDescriptorSet>& _descriptors) const;
	void ResetPool();

private:
	Device& m_device;
	VkDescriptorPool m_descriptorPool;

	friend class DescriptorWriter;
};

//////////////////////////////////////////////////////////////////////////
class VESPERENGINE_API DescriptorWriter
{
public:
	DescriptorWriter(DescriptorSetLayout& _setLayout, DescriptorPool& _pool);

	DescriptorWriter& WriteBuffer(uint32 _binding, VkDescriptorBufferInfo* _bufferInfo, uint32 _count = 1);
	DescriptorWriter& WriteImage(uint32 _binding, VkDescriptorImageInfo* _imageInfo, uint32 _count = 1);

	bool Build(VkDescriptorSet& _set);
	void Overwrite(VkDescriptorSet& _set);

private:
	DescriptorSetLayout& m_setLayout;
	DescriptorPool& m_pool;
	std::vector<VkWriteDescriptorSet> m_writes;
};

VESPERENGINE_NAMESPACE_END
