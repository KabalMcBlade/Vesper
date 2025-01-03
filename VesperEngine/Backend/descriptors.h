#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "Backend/device.h"

#include <unordered_map>
#include <memory>
#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

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
			uint32 _count = 1);
		std::unique_ptr<DescriptorSetLayout> Build() const;

		// convenient conversion operator
// 		operator Builder& () const
// 		{
// 			return Build();
// 		}

	private:
		Device& m_device;
		std::unordered_map<uint32, VkDescriptorSetLayoutBinding> m_bindings{};
	};

	DescriptorSetLayout(Device& _device, std::unordered_map<uint32, VkDescriptorSetLayoutBinding> _bindings);
	~DescriptorSetLayout();
	DescriptorSetLayout(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

	VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

private:
	Device& m_device;
	VkDescriptorSetLayout m_descriptorSetLayout;
	std::unordered_map<uint32, VkDescriptorSetLayoutBinding> m_bindings;

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

	bool AllocateDescriptorSet(const VkDescriptorSetLayout _descriptorSetLayout, VkDescriptorSet& _descriptor) const;
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

	DescriptorWriter& WriteBuffer(uint32 _binding, VkDescriptorBufferInfo* _bufferInfo);
	DescriptorWriter& WriteImage(uint32 _binding, VkDescriptorImageInfo* _imageInfo);

	bool Build(VkDescriptorSet& _set);
	void Overwrite(VkDescriptorSet& _set);

private:
	DescriptorSetLayout& m_setLayout;
	DescriptorPool& m_pool;
	std::vector<VkWriteDescriptorSet> m_writes;
};

VESPERENGINE_NAMESPACE_END
