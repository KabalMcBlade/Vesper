// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\device.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "vma/vk_mem_alloc.h"

#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class WindowHandle;

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

struct QueueFamilyIndices 
{
	uint32 GraphicsFamily;
	uint32 PresentFamily;
	bool GraphicsFamilyHasValue = false;
	bool PresentFamilyHasValue = false;
	bool IsComplete() { return GraphicsFamilyHasValue && PresentFamilyHasValue; }
};

extern PFN_vkCmdSetCullModeEXT vkCmdSetCullModeEXT;
extern PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT;

class VESPERENGINE_API Device final
{
public:
	Device(WindowHandle& _window);
	~Device();

	// Not copyable or movable
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
	Device(Device&&) = delete;
	Device& operator=(Device&&) = delete;

	VESPERENGINE_INLINE const VkCommandPool GetCommandPool() const { return m_commandPool; }
	VESPERENGINE_INLINE const VkDevice GetDevice() const { return m_device; }
	VESPERENGINE_INLINE const VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
	VESPERENGINE_INLINE const VkSurfaceKHR GetSurface() const { return m_surface; }
	VESPERENGINE_INLINE const VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
	VESPERENGINE_INLINE const VkQueue GetPresentQueue() const { return m_presentQueue; }
	VESPERENGINE_INLINE const VmaAllocator GetAllocator() const { return m_allocator; }
	VESPERENGINE_INLINE const VkPhysicalDeviceProperties& GetProperties() const { return m_properties; }
	VESPERENGINE_INLINE const VkPhysicalDeviceLimits& GetLimits() const { return m_properties.limits; }

	VESPERENGINE_INLINE const bool IsBindlessResourcesSupported() const { return m_bIsBindlessResourcesSupported; }

	SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(m_physicalDevice); }
	uint32 FindMemoryType(uint32 _typeFilter, VkMemoryPropertyFlags _properties);
	QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(m_physicalDevice); }
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features);

	// Buffer Helper Functions
	void CreateBuffer(
		VkDeviceSize _size,
		VkBufferUsageFlags _bufferUsage,
		VmaMemoryUsage _memoryUsage,
		VmaAllocationCreateFlags _flags,
		VkBuffer& _buffer,
		VmaAllocation& _allocation,
		bool _isPersistent = false);	// if true, creates a dynamic memory, such the Uniform Buffer, to use in case of frequent write on CPU and frequent read on GPU.

	void CreateBufferWithAlignment(
		VkDeviceSize _size,
		VkBufferUsageFlags _bufferUsage,
		VmaMemoryUsage _memoryUsage,
		VmaAllocationCreateFlags _flags,
		VkBuffer& _buffer,
		VmaAllocation& _allocation,
		VkDeviceSize _minAlignment = 1,	// vertex and index buffer does not need alignment, so is 1, uniform buffer instead, for instance, need it
		bool _isPersistent = false);	// if true, creates a dynamic memory, such the Uniform Buffer, to use in case of frequent write on CPU and frequent read on GPU.

	void CreateImageWithInfo(
		const VkImageCreateInfo& _imageInfo,
		VkImage& _image,
		VmaAllocation& _allocation,
		VmaMemoryUsage _memoryUsage = VMA_MEMORY_USAGE_AUTO);


	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer _commandBuffer);
	// The below methods can be executed all withing the same command buffer, so they can be all executed in one command together if need it
	void CopyBuffer(VkCommandBuffer _commandBuffer, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);
	void CopyBufferToImage(VkCommandBuffer _commandBuffer, VkBuffer _buffer, VkImage _image, uint32 _width, uint32 _height, uint32 _layerCount = 1, uint32 _mipLevel = 1);
	void CopyImageToBuffer(VkCommandBuffer _commandBuffer, VkImage _image, VkBuffer _buffer, uint32 _width, uint32 _height, uint32 _layerCount = 1, uint32 _mipLevel = 1);
	void TransitionImageLayout(VkCommandBuffer _commandBuffer, VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout, uint32 _baseLayerIndex = 0, uint32 _layerCount = 1, uint32 _mipLevel = 1);
	// The below methods are executed withing a single time command buffer, so they are atomic
	void CopyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);
	void CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32 _width, uint32 _height, uint32 _layerCount = 1, uint32 _mipLevel = 1);
	void CopyImageToBuffer(VkImage _image, VkBuffer _buffer, uint32 _width, uint32 _height, uint32 _layerCount = 1, uint32 _mipLevel = 1);
	void CopyImage(VkCommandBuffer _commandBuffer, VkImage _srcImage, VkImage _dstImage, uint32 _width, uint32 _height, uint32 _srcBaseLayerIndex = 0, uint32 _dstBaseLayerIndex = 0, uint32 _layerCount = 1);
	void CopyImage(VkImage _srcImage, VkImage _dstImage, uint32 _width, uint32 _height, uint32 _srcBaseLayerIndex = 0, uint32 _dstBaseLayerIndex = 0, uint32 _layerCount = 1);
	void TransitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout, uint32 _baseLayerIndex = 0, uint32 _layerCount = 1, uint32 _mipLevel = 1);

private:
	// VMA memory settings!
	bool VK_KHR_get_memory_requirements2_enabled = false;
	bool VK_KHR_get_physical_device_properties2_enabled = false;
	bool VK_KHR_dedicated_allocation_enabled = false;
	bool VK_KHR_bind_memory2_enabled = false;
	bool VK_EXT_memory_budget_enabled = false;
	bool VK_AMD_device_coherent_memory_enabled = false;
	bool VK_KHR_buffer_device_address_enabled = false;
	bool VK_EXT_memory_priority_enabled = false;
	bool VK_EXT_debug_utils_enabled = false;
	bool VK_EXT_descriptor_indexing_extension_enabled = false;
	bool VK_EXT_extended_dynamic_state2_enabled = false;

	bool m_bIsBindlessResourcesSupported = false;

private:
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateCommandPool();
	void CreateVma();

	void RecordCopyBuffer(VkCommandBuffer _commandBuffer, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);
	void RecordCopyBufferToImage(VkCommandBuffer _commandBuffer, VkBuffer _buffer, VkImage _image, uint32 _width, uint32 _height, uint32 _layerCount, uint32 _mipLevel);
	void RecordCopyImageToBuffer(VkCommandBuffer _commandBuffer, VkImage _image, VkBuffer _buffer, uint32 _width, uint32 _height, uint32 _layerCount, uint32 _mipLevel);
	void RecordCopyImage(VkCommandBuffer _commandBuffer, VkImage _srcImage, VkImage _dstImage, uint32 _width, uint32 _height, uint32 _srcBaseLayerIndex, uint32 _dstBaseLayerIndex, uint32 _layerCount);
	void RecordTransitionImageLayout(VkCommandBuffer _commandBuffer, VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout, uint32 _baseLayerIndex, uint32 _layerCount, uint32 _mipLevel);

	// helper functions
	bool IsDeviceSuitable(VkPhysicalDevice _device);
	bool CcheckValidationLayerSupport();
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice _device);
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& _createInfo);
	void HasRequiredInstanceExtensions();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice _device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice _device);
	constexpr uint32 GetVulkanApiVersion();

private:
	WindowHandle& m_window;
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkCommandPool m_commandPool;

	VkDevice m_device;
	VkSurfaceKHR m_surface;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkPhysicalDeviceProperties m_properties;

	VmaAllocator m_allocator;

	const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> m_deviceExtensions = { 
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,			// Swapchain support
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,	// Bindless binding support
		VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME  // Extended dynamic state 2
	};
};

VESPERENGINE_NAMESPACE_END
