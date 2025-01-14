// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Backend\device.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Backend/device.h"

#include "Utility/logger.h"

#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

//#define VMA_STATIC_VULKAN_FUNCTIONS VK_TRUE
//#define VMA_DEBUG_GLOBAL_MUTEX VK_TRUE
//#define VMA_RECORDING_ENABLED PICOVK_ENABLE_VALIDATION
//#define VMA_VULKAN_VERSION 1003000
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"


VESPERENGINE_NAMESPACE_BEGIN


// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT _messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* _pCallbackData,
	void* _pUserData)
{
	std::cerr << "validation layer: " << _pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance _instance,
	const VkDebugUtilsMessengerCreateInfoEXT* _pCreateInfo,
	const VkAllocationCallbacks* _pAllocator,
	VkDebugUtilsMessengerEXT* _pDebugMessenger) 
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		_instance,
		"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(_instance, _pCreateInfo, _pAllocator, _pDebugMessenger);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance _instance, VkDebugUtilsMessengerEXT _debugMessenger, const VkAllocationCallbacks* _pAllocator) 
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) 
	{
		func(_instance, _debugMessenger, _pAllocator);
	}
}

// class member functions
Device::Device(WindowHandle& _window) 
	: m_window{ _window } 
{
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateCommandPool();
	CreateVma();
}

Device::~Device() 
{
	vmaDestroyAllocator(m_allocator);

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroyDevice(m_device, nullptr);

	if (m_window.IsValidationLayersEnabled()) {
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

void Device::CreateInstance()
{
	if (m_window.IsValidationLayersEnabled() && !CcheckValidationLayerSupport()) 
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "LittleVulkanEngine App";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Vesper Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = GetVulkanApiVersion();

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = m_window.GetRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (m_window.IsValidationLayersEnabled()) 
	{
		createInfo.enabledLayerCount = static_cast<uint32>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create instance!");
	}

	HasRequiredInstanceExtensions();
}

void Device::PickPhysicalDevice() 
{
	uint32 deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
	if (deviceCount == 0) 
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	LOG(Logger::INFO, "Device count: ", deviceCount);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	for (const auto& device : devices) 
	{
		if (IsDeviceSuitable(device)) 
		{
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE) 
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);

	LOG(Logger::INFO, "Physical device: ", m_properties.deviceName);
	LOG_NL();

	LOG(Logger::INFO, "Next Gen Hardware limits: ", m_properties.deviceName);
	LOG(Logger::INFO, "\t", "maxDescriptorSetSampledImages: ", m_properties.limits.maxDescriptorSetSampledImages);
	LOG(Logger::INFO, "\t", "maxPerStageDescriptorSamplers: ", m_properties.limits.maxPerStageDescriptorSamplers);
	LOG(Logger::INFO, "\t", "maxPerStageDescriptorUniformBuffers: ", m_properties.limits.maxPerStageDescriptorUniformBuffers);
	LOG_NL();
}

void Device::CreateLogicalDevice() 
{
	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32> uniqueQueueFamilies = { indices.GraphicsFamily, indices.PresentFamily };

	float queuePriority = 1.0f;
	for (uint32 queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	// EXT features
	VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures = {};
	indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	indexingFeatures.runtimeDescriptorArray = VK_TRUE;
	indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
	indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
	indexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
	indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
// 	indexingFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
// 	indexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
// 	indexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE; 	

	VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.features = deviceFeatures;  // Include base features like samplerAnisotropy

	if (m_bIsBindlessResourcesSupported)
	{
		deviceFeatures2.pNext = &indexingFeatures;
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.enabledExtensionCount = static_cast<uint32>(m_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

	// validation layers
	if (m_window.IsValidationLayersEnabled())
	{
		createInfo.enabledLayerCount = static_cast<uint32>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

	// Use features2 instead of pEnabledFeatures
	createInfo.pEnabledFeatures = nullptr;
	createInfo.pNext = &deviceFeatures2;

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(m_device, indices.GraphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.PresentFamily, 0, &m_presentQueue);
}

void Device::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindPhysicalQueueFamilies();

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily;
	poolInfo.flags =
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

void Device::CreateVma()
{
	VmaAllocatorCreateInfo allocatorInfo = {};

	allocatorInfo.physicalDevice = m_physicalDevice;
	allocatorInfo.device = m_device;
	allocatorInfo.instance = m_instance;
	allocatorInfo.vulkanApiVersion = GetVulkanApiVersion();

	if (VK_KHR_dedicated_allocation_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
	}
	if (VK_KHR_bind_memory2_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
	}
#if !defined(VMA_MEMORY_BUDGET) || VMA_MEMORY_BUDGET == 1
	if (VK_EXT_memory_budget_enabled && (GetVulkanApiVersion() >= VK_API_VERSION_1_1 || VK_KHR_get_physical_device_properties2_enabled))
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	}
#endif
	if (VK_AMD_device_coherent_memory_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
	}
	if (VK_KHR_buffer_device_address_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	}
#if !defined(VMA_MEMORY_PRIORITY) || VMA_MEMORY_PRIORITY == 1
	if (VK_EXT_memory_priority_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
	}
#endif

#if VMA_DYNAMIC_VULKAN_FUNCTIONS
	static VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	allocatorInfo.pVulkanFunctions = &vulkanFunctions;
#endif

	// Uncomment to enable recording to CSV file.
	/*
	static VmaRecordSettings recordSettings = {};
	recordSettings.pFilePath = "VulkanSample.csv";
	allocatorInfo.pRecordSettings = &recordSettings;
	*/

	// Uncomment to enable HeapSizeLimit.
	/*
	static std::array<VkDeviceSize, VK_MAX_MEMORY_HEAPS> heapSizeLimit;
	std::fill(heapSizeLimit.begin(), heapSizeLimit.end(), VK_WHOLE_SIZE);
	heapSizeLimit[0] = 512ull * 1024 * 1024;
	allocatorInfo.pHeapSizeLimit = heapSizeLimit.data();
	*/

	if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create VMA allocator!");
	}
}

void Device::CreateSurface() 
{ 
	m_window.CreateWindowSurface(m_instance, &m_surface); 
}

bool Device::IsDeviceSuitable(VkPhysicalDevice device) 
{
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	// Check for bindless support
	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
	descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = &descriptorIndexingFeatures;

	// Get physical device features including descriptor indexing
	vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);

	m_bIsBindlessResourcesSupported = VK_EXT_descriptor_indexing_extension_enabled &&
		descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing &&
		descriptorIndexingFeatures.runtimeDescriptorArray &&
		descriptorIndexingFeatures.descriptorBindingPartiallyBound &&
		descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount &&
		descriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind &&
		descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind &&
		descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind &&
		descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind &&
		descriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind &&
		descriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind;

	return indices.IsComplete() 
		&& extensionsSupported 
		&& swapChainAdequate
		&& supportedFeatures.samplerAnisotropy;
		//&& m_bIsBindlessResourcesSupported;	// not mandatory, the system can run either way (if the shader support)
}

void Device::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& _createInfo) 
{
	_createInfo = {};
	_createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	_createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	_createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	_createInfo.pfnUserCallback = DebugCallback;
	_createInfo.pUserData = nullptr;  // Optional
}

void Device::SetupDebugMessenger() 
{
	if (!m_window.IsValidationLayersEnabled()) 
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

bool Device::CcheckValidationLayerSupport()
{
	uint32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : m_validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) 
		{
			if (strcmp(layerName, layerProperties.layerName) == 0) 
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) 
		{
			return false;
		}
	}

	return true;
}

void Device::HasRequiredInstanceExtensions()
{
	uint32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	LOG(Logger::INFO, "Available extensions:");

	std::unordered_set<std::string> available;
	for (const auto& extension : extensions)
	{
		LOG(Logger::INFO, "\t", extension.extensionName);

		available.insert(extension.extensionName);
	}

	LOG(Logger::INFO, "Required extensions:");

	auto requiredExtensions = m_window.GetRequiredExtensions();
	for (const auto& required : requiredExtensions)
	{
		LOG(Logger::INFO, "\t", required);

		if (available.find(required) == available.end()) 
		{
			throw std::runtime_error("Missing required extensions");
		}
	}
}

bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice _device) 
{
	uint32 extensionCount;
	vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
		_device,
		nullptr,
		&extensionCount,
		availableExtensions.data());

	std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		if (strcmp(extension.extensionName, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) == 0)
		{
			if (GetVulkanApiVersion() == VK_API_VERSION_1_0)
			{
				VK_KHR_get_memory_requirements2_enabled = true;
			}
		}
		else if (strcmp(extension.extensionName, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) == 0)
		{
			if (GetVulkanApiVersion() == VK_API_VERSION_1_0)
			{
				VK_KHR_dedicated_allocation_enabled = true;
			}
		}
		else if (strcmp(extension.extensionName, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME) == 0)
		{
			if (GetVulkanApiVersion() == VK_API_VERSION_1_0)
			{
				VK_KHR_bind_memory2_enabled = true;
			}
		}
		else if (strcmp(extension.extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
		{
			VK_EXT_memory_budget_enabled = true;
		}
		else if (strcmp(extension.extensionName, VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME) == 0)
		{
			VK_AMD_device_coherent_memory_enabled = true;
		}
		else if (strcmp(extension.extensionName, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0)
		{
			if (GetVulkanApiVersion() < VK_API_VERSION_1_2)
			{
				VK_KHR_buffer_device_address_enabled = true;
			}
		}
		else if (strcmp(extension.extensionName, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) == 0)
		{
			VK_EXT_memory_priority_enabled = true;
		}
		else if (strcmp(extension.extensionName, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0)
		{
			VK_EXT_descriptor_indexing_extension_enabled = true;
		}

		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice _device)
{
	QueueFamilyIndices indices;

	uint32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.GraphicsFamily = i;
			indices.GraphicsFamilyHasValue = true;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, m_surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.PresentFamily = i;
			indices.PresentFamilyHasValue = true;
		}
		if (indices.IsComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

SwapChainSupportDetails Device::QuerySwapChainSupport(VkPhysicalDevice _device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, m_surface, &details.Capabilities);

	uint32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_device, m_surface, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_device, m_surface, &formatCount, details.Formats.data());
	}

	uint32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			_device,
			m_surface,
			&presentModeCount,
			details.PresentModes.data());
	}
	return details;
}

constexpr uint32 Device::GetVulkanApiVersion()
{
#if VMA_VULKAN_VERSION == 1003000
	return VK_API_VERSION_1_3;
#elif VMA_VULKAN_VERSION == 1002000
	return VK_API_VERSION_1_2;
#elif VMA_VULKAN_VERSION == 1001000
	return VK_API_VERSION_1_1;
#elif VMA_VULKAN_VERSION == 1000000
	return VK_API_VERSION_1_0;
#else
#error Invalid VMA_VULKAN_VERSION.
	return UINT32_MAX;
#endif
}

VkFormat Device::FindSupportedFormat(const std::vector<VkFormat>& _candidates, VkImageTiling _tiling, VkFormatFeatureFlags _features) 
{
	for (VkFormat format : _candidates) 
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

		if (_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & _features) == _features)
		{
			return format;
		}
		else if (_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & _features) == _features)
		{
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

uint32 Device::FindMemoryType(uint32 _typeFilter, VkMemoryPropertyFlags _properties) 
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
	for (uint32 i = 0; i < memProperties.memoryTypeCount; ++i) 
	{
		if ((_typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & _properties) == _properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void Device::CreateBuffer(
	VkDeviceSize _size,
	VkBufferUsageFlags _bufferUsage,
	VmaMemoryUsage _memoryUsage,
	VmaAllocationCreateFlags _flags,
	VkBuffer& _buffer,
	VmaAllocation& _allocation,
	bool _isPersistent)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = _size;
	bufferInfo.usage = _bufferUsage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo createInfo{};
	createInfo.usage = _memoryUsage;	//VMA_MEMORY_USAGE_AUTO;//VMA_MEMORY_USAGE_AUTO_PREFER_HOST;//VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	createInfo.flags = _flags;//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; //VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
// 	createInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
// 	createInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
// 	createInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	if (_isPersistent)
	{
		createInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
		// 		createInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
		// 			VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
		// 			VMA_ALLOCATION_CREATE_MAPPED_BIT;
	}
	
	if (vmaCreateBuffer(m_allocator, &bufferInfo, &createInfo, &_buffer, &_allocation, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create, allocate and bind buffer!");
	}

	/*
	if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &_buffer) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, _buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, _properties);

	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(m_device, _buffer, _bufferMemory, 0);
	*/
}

void Device::CreateBufferWithAlignment(
	VkDeviceSize _size,
	VkBufferUsageFlags _bufferUsage,
	VmaMemoryUsage _memoryUsage,
	VmaAllocationCreateFlags _flags,
	VkBuffer& _buffer,
	VmaAllocation& _allocation,
	VkDeviceSize _minAlignment /*= 1*/,	// vertex and index buffer does not need alignment, so is 1, uniform buffer instead, for instance, need it
	bool _isPersistent /*= false*/)		// if true, creates a dynamic memory, such the Uniform Buffer, to use in case of frequent write on CPU and frequent read on GPU.
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = _size;
	bufferInfo.usage = _bufferUsage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo createInfo{};
	createInfo.usage = _memoryUsage;	//VMA_MEMORY_USAGE_AUTO;//VMA_MEMORY_USAGE_AUTO_PREFER_HOST;//VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	createInfo.flags = _flags;//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; //VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
// 	createInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
// 	createInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
// 	createInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	if (_isPersistent)
	{
		createInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
// 		createInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
// 			VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
// 			VMA_ALLOCATION_CREATE_MAPPED_BIT;
	}
	if (vmaCreateBufferWithAlignment(m_allocator, &bufferInfo, &createInfo, _minAlignment, &_buffer, &_allocation, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create, allocate and bind buffer!");
	}
}

void Device::CreateImageWithInfo(
	const VkImageCreateInfo& _imageInfo,
	VkImage& _image,
	VmaAllocation& _allocation,
	VmaMemoryUsage _memoryUsage)
{
	VmaAllocationCreateInfo createInfo{};
	createInfo.usage = _memoryUsage;

	if (vmaCreateImage(m_allocator, &_imageInfo, &createInfo, &_image, &_allocation, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create, allocate and bind image!");
	}

	/*
	if (vkCreateImage(m_device, &_imageInfo, nullptr, &_image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, _image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, _properties);

	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &_imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	if (vkBindImageMemory(m_device, _image, _imageMemory, 0) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to bind image memory!");
	}
	*/
}

VkCommandBuffer Device::BeginSingleTimeCommands() 
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void Device::EndSingleTimeCommands(VkCommandBuffer _commandBuffer)
{
	vkEndCommandBuffer(_commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &_commandBuffer);

	/*
	vkEndCommandBuffer(_commandBuffer);

	// Create a fence for synchronization
	VkFence fence;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(m_device, &fenceInfo, nullptr, &fence);

	// Submit the command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, fence);

	// Wait for the fence
	vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);

	// Destroy the fence
	vkDestroyFence(m_device, fence, nullptr);

	// Free the command buffer
	vkFreeCommandBuffers(m_device, m_commandPool, 1, &_commandBuffer);
	*/
}



void Device::RecordCopyBuffer(VkCommandBuffer _commandBuffer, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
{
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = _size;
	vkCmdCopyBuffer(_commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);
}

void Device::RecordCopyBufferToImage(VkCommandBuffer _commandBuffer, VkBuffer _buffer, VkImage _image, uint32 _width, uint32 _height, uint32 _layerCount, uint32 _mipLevel)
{
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = _mipLevel - 1;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = _layerCount;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { _width, _height, 1 };
	vkCmdCopyBufferToImage(_commandBuffer, _buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Device::RecordCopyImageToBuffer(VkCommandBuffer _commandBuffer, VkImage _image, VkBuffer _buffer, uint32 _width, uint32 _height, uint32 _layerCount, uint32 _mipLevel)
{
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = _mipLevel - 1;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = _layerCount;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { _width, _height, 1 };
	vkCmdCopyImageToBuffer(_commandBuffer, _image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _buffer, 1, &region);
}

/**
 * EXAMPLE TRANSITIONS:
 *
 * Transition for Rendering:
 * m_device.TransitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
 *
 * Transition for Saving / copy to staging buffer:
 * m_device.TransitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
 *
 * Transition for Sampling in the Shader
 * m_device.TransitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
 *
 * Transition for general Destination
 * m_device.TransitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
 *
 */
void Device::RecordTransitionImageLayout(VkCommandBuffer _commandBuffer, VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout, uint32 _layerCount, uint32 _mipLevel)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = _oldLayout;
	barrier.newLayout = _newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = _image;

	// Use format to determine the aspect mask
	if (_format == VK_FORMAT_D32_SFLOAT || _format == VK_FORMAT_D32_SFLOAT_S8_UINT || _format == VK_FORMAT_D24_UNORM_S8_UINT)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		// Add stencil aspect if the format has a stencil component
		if (_format == VK_FORMAT_D32_SFLOAT_S8_UINT || _format == VK_FORMAT_D24_UNORM_S8_UINT)
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Default to color images
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = _mipLevel;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = _layerCount;

	VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	// Handle layout transitions
	if (_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	else
	{
		throw std::invalid_argument("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		_commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}


void Device::CopyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	RecordCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, _size);
	EndSingleTimeCommands(commandBuffer);
}

void Device::CopyBuffer(VkCommandBuffer _commandBuffer, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
{
	RecordCopyBuffer(_commandBuffer, _srcBuffer, _dstBuffer, _size);
}

void Device::CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32 _width, uint32 _height, uint32 _layerCount, uint32 _mipLevel)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	RecordCopyBufferToImage(commandBuffer, _buffer, _image, _width, _height, _layerCount, _mipLevel);
	EndSingleTimeCommands(commandBuffer);
}

void Device::CopyBufferToImage(VkCommandBuffer _commandBuffer, VkBuffer _buffer, VkImage _image, uint32 _width, uint32 _height, uint32 _layerCount, uint32 _mipLevel)
{
	RecordCopyBufferToImage(_commandBuffer, _buffer, _image, _width, _height, _layerCount, _mipLevel);
}

void Device::CopyImageToBuffer(VkImage _image, VkBuffer _buffer, uint32 _width, uint32 _height, uint32 _layerCount, uint32 _mipLevel)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	RecordCopyImageToBuffer(commandBuffer, _image, _buffer, _width, _height, _layerCount, _mipLevel);
	EndSingleTimeCommands(commandBuffer);
}

void Device::CopyImageToBuffer(VkCommandBuffer _commandBuffer, VkImage _image, VkBuffer _buffer, uint32 _width, uint32 _height, uint32 _layerCount, uint32 _mipLevel)
{
	RecordCopyImageToBuffer(_commandBuffer, _image, _buffer, _width, _height, _layerCount, _mipLevel);
}

void Device::TransitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout, uint32 _layerCount, uint32 _mipLevel)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	RecordTransitionImageLayout(commandBuffer, _image, _format, _oldLayout, _newLayout, _layerCount, _mipLevel);
	EndSingleTimeCommands(commandBuffer);
}

void Device::TransitionImageLayout(VkCommandBuffer _commandBuffer, VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout, uint32 _layerCount, uint32 _mipLevel)
{
	RecordTransitionImageLayout(_commandBuffer, _image, _format, _oldLayout, _newLayout, _layerCount, _mipLevel);
}

VESPERENGINE_NAMESPACE_END
