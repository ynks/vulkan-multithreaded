module;
#include <algorithm>
#include <print>
#include <vulkan/vulkan_raii.hpp>
#include <expected>

module vulkan.device;

import vulkan.instance;
import window;

namespace vulkan {

Device::Device() {
	if (m_thisDevice) {
		throw std::runtime_error("One Vulkan Device already exists");
	}

	m_thisDevice = this;
	PickPhysicalDevice();
	CreateLogicalDevice();
}

Device* Device::device() {
	if (!m_thisDevice) {
		throw std::runtime_error("Trying to access Vulkan Device but it doesn't exist yet");
	}

	return m_thisDevice;
}

std::expected<unsigned, DeviceError> Device::GetDeviceScore(const vk::raii::PhysicalDevice& device) {
	auto properties = device.getProperties();
	auto features = device.getFeatures();

	// Hard requirements - device must support these
	if (!features.geometryShader) return std::unexpected{DeviceError::eNoGeometryShader};
	if (!features.vertexPipelineStoresAndAtomics) return std::unexpected{DeviceError::eNoVertexShader};
	if (!features.fragmentStoresAndAtomics) return std::unexpected{DeviceError::eNoFragmentShader};
	
	// Check for compute shader support via queue families
	auto queue_families = device.getQueueFamilyProperties();
	bool has_compute = false;
	bool has_graphics = false;
	for (const auto& queueFamily : queue_families) {
		if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) has_compute = true;
		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) has_graphics = true;
	}
	if (!has_compute) return std::unexpected{DeviceError::eNoComputeShader};
	if (!has_graphics) return std::unexpected{DeviceError::eNoPresentationSupport};

	unsigned score = 0;

	// Device type scoring (discrete GPUs are preferred)
	switch (properties.deviceType) {
		case vk::PhysicalDeviceType::eDiscreteGpu:
			score += 1000;
			break;
		case vk::PhysicalDeviceType::eIntegratedGpu:
			score += 500;
			break;
		case vk::PhysicalDeviceType::eVirtualGpu:
			score += 300;
			break;
		case vk::PhysicalDeviceType::eCpu:
			score += 100;
			break;
		default:
			score += 10;
			break;
	}

	// Image dimensions (higher is better)
	score += properties.limits.maxImageDimension2D / 1000;

	// Memory heap size (larger VRAM is better)
	auto memory_properties = device.getMemoryProperties();
	for (uint32_t i = 0; i < memory_properties.memoryHeapCount; i++) {
		if (memory_properties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
			score += memory_properties.memoryHeaps[i].size / (1024 * 1024 * 1024); // Score per GB
		}
	}

	// Additional feature bonuses
	if (features.tessellationShader) score += 50;
	if (features.multiViewport) score += 20;
	if (features.samplerAnisotropy) score += 30;
	if (features.fillModeNonSolid) score += 10;
	if (features.wideLines) score += 10;

	// Max descriptor sets and viewports
	score += properties.limits.maxBoundDescriptorSets;
	score += properties.limits.maxViewports;
	score += properties.limits.maxColorAttachments;

	std::println("Device \"{0}\" scored {1}", static_cast<std::string>(properties.deviceName), score);

	return score;
}

void Device::PickPhysicalDevice() {
	std::println("Getting physical device...");

	// Get all devices
	auto vk_instance = Instance::get();
	auto devices = vk_instance->enumeratePhysicalDevices();
	if (devices.empty()) throw std::runtime_error("No physical device found");
	std::println("Found {} devices:", devices.size());
	for (const auto& d : devices) std::println("\t{}", static_cast<std::string>(d.getProperties().deviceName));

	// Assign score
	auto bestDevice = std::max_element(devices.begin(), devices.end(),
		[this](const vk::raii::PhysicalDevice& a, const vk::raii::PhysicalDevice& b) {
			auto scoreA = GetDeviceScore(a);
			auto scoreB = GetDeviceScore(b);
			
			if (!scoreA.has_value()) return true;
			if (!scoreB.has_value()) return false;
			
			return scoreA.value() < scoreB.value();
		});

	auto finalScore = GetDeviceScore(*bestDevice);
	if (!finalScore.has_value()) {
		throw std::runtime_error("No suitable physical device found");
	}

	m_physicalDevice = *bestDevice;
	std::println("Physical device \"{0}\" chosen", static_cast<std::string>(m_physicalDevice.getProperties().deviceName));
}

std::expected<uint32_t, DeviceError> Device::GetQueueFamily(vk::QueueFlagBits type) {
	if (m_physicalDevice == nullptr) throw std::runtime_error("Trying to get QueueFamily without a device");

	std::vector queue_family_properties = m_physicalDevice.getQueueFamilyProperties();

	
	// finds the first queue with the target type
	auto target = std::find_if(queue_family_properties.begin(), queue_family_properties.end(), [type](const auto& qfp){
		return qfp.queueFlags & type;
	});
	if (target == queue_family_properties.end()) return std::unexpected(DeviceError::eNoQueueFound);
	return static_cast<uint32_t>(std::distance(queue_family_properties.begin(), target));
}

void Device::CreateLogicalDevice() {
	std::println("Creating logical device...");

	// Get queue families
	std::vector queue_properties = m_physicalDevice.getQueueFamilyProperties();
	uint32_t graphics_index = GetQueueFamily(vk::QueueFlagBits::eGraphics).value();
	std::println("Got graphics queue at {}", graphics_index);

	// Create queues
	float graphics_queue_priority = 0.5f;
	float compute_queue_priority = 0.2f;

	std::vector<vk::DeviceQueueCreateInfo> queues_info = {
		vk::DeviceQueueCreateInfo {
			.queueFamilyIndex = graphics_index,
			.queueCount = 1,
			.pQueuePriorities = &graphics_queue_priority
		}
	};

	// Enable features
	vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extDynamicState{
		.extendedDynamicState = true
	};
	vk::PhysicalDeviceVulkan13Features vk13Features{
		.pNext = &extDynamicState,
		.dynamicRendering = true
	};
	vk::PhysicalDeviceFeatures2 features2{
		.pNext = &vk13Features
	};

	// Enable extensions
	std::vector<const char*> device_extensions = {
		vk::KHRSwapchainExtensionName,
		vk::KHRSpirv14ExtensionName,
		vk::KHRSynchronization2ExtensionName,
		vk::KHRCreateRenderpass2ExtensionName
	};
	std::println("Enabling {} device extensions:", device_extensions.size());
	for (const auto& ext : device_extensions) std::println("\t{}", ext);

	// Create device
	vk::DeviceCreateInfo device_create_info{
		.pNext = &features2,
		.queueCreateInfoCount = static_cast<uint32_t>(queues_info.size()),
		.pQueueCreateInfos = queues_info.data(),
		.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
		.ppEnabledExtensionNames = device_extensions.data()
	};
	m_device = vk::raii::Device(m_physicalDevice, device_create_info);

	// Retrieve queue
	m_graphicsQueue = vk::raii::Queue(m_device, graphics_index, 0);

	std::println("Created Vulkan Device");
}

}
