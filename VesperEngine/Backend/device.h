#pragma once

#include "vulkan/vulkan.h"

#include "Core/core_defines.h"
#include "App/window_handle.h"

#include "vma/vk_mem_alloc.h"


VESPERENGINE_NAMESPACE_BEGIN


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
	VESPERENGINE_INLINE const VkSurfaceKHR GetSurface() const { return m_surface; }
	VESPERENGINE_INLINE const VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
	VESPERENGINE_INLINE const VkQueue GetPresentQueue() const { return m_presentQueue; }
	VESPERENGINE_INLINE const VmaAllocator GetAllocator() const { return m_allocator; }
	VESPERENGINE_INLINE const VkPhysicalDeviceProperties& GetProperties() const { return m_properties; }
	VESPERENGINE_INLINE const VkPhysicalDeviceLimits& GetLimits() const { return m_properties.limits; }

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
		VmaAllocation& _allocation);

	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer _commandBuffer);
	void CopyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);
	void CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32 _width, uint32 _height, uint32 _layerCount);
	void TransitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout);

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
	bool g_SparseBindingEnabled = false;

private:
	void CreateInstance();
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateCommandPool();
	void CreateVma();

	// helper functions
	bool IsDeviceSuitable(VkPhysicalDevice _device);
	bool CcheckValidationLayerSupport();
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice _device);
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& _createInfo);
	void HasRequiredInstanceExtensions();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice _device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice _device);
	constexpr uint32 GetVulkanApiVersion();

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	WindowHandle& m_window;
	VkCommandPool m_commandPool;

	VkDevice m_device;
	VkSurfaceKHR m_surface;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkPhysicalDeviceProperties m_properties;

	VmaAllocator m_allocator;

	const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

VESPERENGINE_NAMESPACE_END
