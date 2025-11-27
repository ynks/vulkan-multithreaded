module;

#include <print>
#include <stdexcept>
#include <vulkan/vulkan_raii.hpp>
import vulkan.device;
import window;

module vulkan.swapchain;

namespace vulkan {

Swapchain* Swapchain::m_instance = nullptr;

Swapchain* Swapchain::swapchain() {
	if (!m_instance) throw std::runtime_error("Trying to access Swapchain but it doesn't exist yet");
	return m_instance;
}

Swapchain* Swapchain::operator()() { return swapchain(); }

Swapchain::Swapchain() {
	if (m_instance != nullptr) { throw std::runtime_error("One Swapchain already exists"); }
	m_instance = this;

	std::println("Creating swapchain...");
	// Getting device capabilitites
	auto* physical_device = device()->physicalDevice();
	auto* surface = window()->surface();

	auto surface_capabilities = physical_device->getSurfaceCapabilitiesKHR(*surface);

	std::vector available_formats = physical_device->getSurfaceFormatsKHR(*surface);
	std::println("Obtained {} available formats", available_formats.size());
	std::vector available_present_modes = physical_device->getSurfacePresentModesKHR(*surface);
	std::println("Obtained {} available present modes", available_present_modes.size());

	// Creating the Swapchain
	m_surfaceFormat = ChooseSurfaceFormat(available_formats);
	m_presentMode = ChoosePresentMode(available_present_modes);
	m_extent = ChooseExtent(surface_capabilities);
	uint32_t min_image_count = std::max(3u, surface_capabilities.minImageCount);
	if (surface_capabilities.maxImageCount > 0 && min_image_count > surface_capabilities.maxImageCount) {
		min_image_count = surface_capabilities.maxImageCount;
	}
	uint32_t image_count = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount) {
		image_count = surface_capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR swapchain_create_info {
		.flags = vk::SwapchainCreateFlagsKHR(),
		.surface = *surface,
		.minImageCount = min_image_count,
		.imageFormat = m_surfaceFormat.format,
		.imageColorSpace = m_surfaceFormat.colorSpace,
		.imageExtent = m_extent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surface_capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = m_presentMode,
		.clipped = true,
		.oldSwapchain = nullptr
	};

	const std::array queue_family_indices = { Device::graphicsIndex(), Device::presentIndex() };

	if (Device::graphicsIndex() != Device::presentIndex()) {
		std::println("Graphics index is not the same as Present index, changing Image Sharing Mode to Concurrent");
		swapchain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchain_create_info.queueFamilyIndexCount = queue_family_indices.size();
		swapchain_create_info.pQueueFamilyIndices = queue_family_indices.data();
	}

	m_swapchain = vk::raii::SwapchainKHR(*Device::get(), swapchain_create_info);
	m_images = m_swapchain.getImages();
	m_format = m_surfaceFormat.format;

	CreateImageViews();

	std::println("Created Swapchain");
}

vk::SurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
	// TODO: make it choose the best one (10bit HDR) over 8bit sRGB
	auto result = std::ranges::find_if(formats, [](const auto& available) {
		// finds the first one that supports 8bit BRGA sRGB
		if (available.format == vk::Format::eB8G8R8A8Srgb && available.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return true;
		}
		return false;
	});

	if (result == formats.end()) {
		// If we cannot find the type we want, just go with the first type of the list as backup
		return formats[0];
	}

	return *result;
}

vk::PresentModeKHR Swapchain::ChoosePresentMode(const std::vector<vk::PresentModeKHR>& modes) {
	// TODO: The user should be able to choose this in the settings
	// I'm forcing V-sync for now since I value the battery performance of my laptop
	// if (modes.contains(vk::PresentModeKHR::eMailbox) {
	// 	return vk::PresentModeKHR::eMailbox;
	// }

	// Force V-sync if nothing else is available
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::ChooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	const auto [width, height] = window()->framebufferSize();
	return {
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

void Swapchain::CreateImageViews() {
	// Remove every existing view before starting
	m_imageViews.clear();

	// Create view
	vk::ImageViewCreateInfo view_info {
		.viewType = vk::ImageViewType::e2D,
		.format = m_format,
		.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
	};

	for (auto image : m_images) {
		view_info.image = image;
		m_imageViews.emplace_back(*Device::get(), view_info);
	}

	std::println("Created {} Image Views", m_imageViews.size());
}

}
