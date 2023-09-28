#include "pch.h"

#include "Backend/device.h"

#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

//#define VMA_STATIC_VULKAN_FUNCTIONS VK_TRUE
//#define VMA_DEBUG_GLOBAL_MUTEX VK_TRUE
//#define VMA_RECORDING_ENABLED PICOVK_ENABLE_VALIDATION
//#define VMA_VULKAN_VERSION 1003000
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>


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

#ifdef _DEBUG
	std::cout << "Device count: " << deviceCount << std::endl;
#endif

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

#ifdef _DEBUG
	std::cout << "physical device: " << m_properties.deviceName << std::endl;
#endif
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

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32>(m_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

	// might not really be necessary anymore because device specific validation layers
	// have been deprecated
	if (m_window.IsValidationLayersEnabled())
	{
		createInfo.enabledLayerCount = static_cast<uint32>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

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

	return indices.IsComplete() && extensionsSupported && swapChainAdequate &&
		supportedFeatures.samplerAnisotropy;
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

#ifdef _DEBUG
	std::cout << "available extensions:" << std::endl;
#endif
	std::unordered_set<std::string> available;
	for (const auto& extension : extensions)
	{
#ifdef _DEBUG
		std::cout << "\t" << extension.extensionName << std::endl;
#endif
		available.insert(extension.extensionName);
	}

#ifdef _DEBUG
	std::cout << "required extensions:" << std::endl;
#endif
	auto requiredExtensions = m_window.GetRequiredExtensions();
	for (const auto& required : requiredExtensions)
	{
#ifdef _DEBUG
		std::cout << "\t" << required << std::endl;
#endif
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
	VmaAllocationCreateFlags flags,
	VkBuffer& _buffer,
	VmaAllocation& _allocation)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = _size;
	bufferInfo.usage = _bufferUsage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo createInfo{};
	createInfo.usage = _memoryUsage;	//VMA_MEMORY_USAGE_AUTO;//VMA_MEMORY_USAGE_AUTO_PREFER_HOST;//VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	createInfo.flags = flags;//VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; //VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
// 	createInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
// 	createInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
// 	createInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

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


void Device::CreateImageWithInfo(
	const VkImageCreateInfo& _imageInfo,
	VkImage& _image,
	VmaAllocation& _allocation)
{
	VmaAllocationCreateInfo createInfo{};
	createInfo.usage = VMA_MEMORY_USAGE_AUTO;

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
}

void Device::CopyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size) 
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;  // Optional
	copyRegion.dstOffset = 0;  // Optional
	copyRegion.size = _size;
	vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);
	
	EndSingleTimeCommands(commandBuffer);
}

void Device::CopyBufferToImage(VkBuffer _buffer, VkImage _image, uint32 _width, uint32 _height, uint32 _layerCount) 
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = _layerCount;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { _width, _height, 1 };
	
	vkCmdCopyBufferToImage(
		commandBuffer,
		_buffer,
		_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);
	EndSingleTimeCommands(commandBuffer);
}

VESPERENGINE_NAMESPACE_END