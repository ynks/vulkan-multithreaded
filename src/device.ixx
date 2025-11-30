/// @file device.ixx
/// @author Xein
/// @date 26-Nov-2025

module;
#include <vulkan/vulkan_raii.hpp>
#include <expected>

export module vulkan.device;

import vulkan.instance;

namespace vulkan {

export enum class DeviceError {
	eNoGeometryShader,
	eNoVertexShader,
	eNoFragmentShader,
	eNoComputeShader,
	eNoPresentationSupport,
	eNoQueueFound
};

export class Device {
public:
	Device();
	static Device* device();

	[[nodiscard]] /// @brief Exposes vk::raii::Device
	static vk::raii::Device& get() { return device()->m_device; }

	[[nodiscard]] /// @brief Get physical device
	static vk::raii::PhysicalDevice& physicalDevice() { return device()->m_physicalDevice; }

	[[nodiscard]] /// @brief Get graphics queue
	static vk::raii::Queue& queue() { return device()->m_graphicsQueue; }

	[[nodiscard]] /// @brief Get present queue
	static vk::raii::Queue& presentQueue() { return device()->m_presentQueue; }

	[[nodiscard]]
	static uint32_t graphicsIndex() { return device()->m_graphicsFamilyIndex; }
	[[nodiscard]]
	static uint32_t presentIndex() { return device()->m_presentFamilyIndex; }

private:
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	std::expected<unsigned, DeviceError> GetDeviceScore(const vk::raii::PhysicalDevice& device);
	std::expected<uint32_t, DeviceError> GetQueueFamily(vk::QueueFlagBits type = vk::QueueFlagBits::eGraphics);

	static Device* m_thisDevice;
	vk::raii::PhysicalDevice m_physicalDevice = nullptr;
	vk::raii::Device m_device = nullptr;
	uint32_t m_graphicsFamilyIndex;
	uint32_t m_presentFamilyIndex;
	vk::raii::Queue m_graphicsQueue = nullptr;
	vk::raii::Queue m_presentQueue = nullptr;
};

/// @brief public wrapper to get the vulkan device
export Device* device() { return Device::device(); }

/// @brief public wrapper to get the vulkan queue
export vk::raii::Queue& queue() { return vulkan::Device::queue(); }

Device* Device::m_thisDevice = nullptr;

}
