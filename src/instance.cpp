module;
#include <array>
#include <GLFW/glfw3.h>
#include <print>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

module vulkan.instance;

namespace vulkan {

#ifdef NDEBUG
constexpr bool VALIDATION_LAYERS_ENABLED = false;
#else
constexpr bool VALIDATION_LAYERS_ENABLED = true;
#endif

constexpr std::array g_validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

/// @brief Sets the logger for Validation Layers
static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT type,
	const vk::DebugUtilsMessengerCallbackDataEXT* callback_data,
	void*
);

Instance::Instance() {
	if (m_thisInstance) {
		throw std::runtime_error("One Vulkan Instance already exists");
	}

	// Create and configure vk::Instance
	m_thisInstance = this;
	CreateInstance();
	SetupDebugMessenger();
}

Instance* Instance::instance() {
	if (!m_thisInstance) {
		throw std::runtime_error("Trying to access Vulkan Instance but it doesn't exist yet");
	}

	// Get pointer to singleton
	return m_thisInstance;
}

void Instance::CreateInstance() {
		std::println("Started Vulkan instance creation...");

		// Contains information about the application
		constexpr vk::ApplicationInfo app_info = {
			.pApplicationName = "CS180 final",
			.applicationVersion = VK_MAKE_VERSION(1, 0 ,0),
			.pEngineName = "Toast Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
				.apiVersion = vk::ApiVersion14
		};

		// Get validation layers
		std::vector<const char*> required_layers;
		if constexpr (VALIDATION_LAYERS_ENABLED) {
			required_layers.assign(g_validationLayers.begin(), g_validationLayers.end());
			std::println("Validation layers enabled:");
			for (const auto& l : required_layers) { std::println("\t{}", l); }
		}

		// Check if the required layers are supported by Vulkan
		auto layer_props = m_context.enumerateInstanceLayerProperties();
		// returns true if any of the layers...
		if (std::ranges::any_of(required_layers, [layer_props](const auto& required){
				// doesn't match with none of the context properties
				return std::ranges::none_of(layer_props, [required](const auto& layer_prop){
					return strcmp(layer_prop.layerName, required) == 0;
				});
		})) {
			throw std::runtime_error("Required Validation Layers are not supported");
		}

		std::println("All validation layers are supported");

		// Get required extensions
		auto required_extensions = GetRequiredExtensions();

		// Contains information about the vulkan instance
		vk::InstanceCreateInfo create_info {
			.pApplicationInfo = &app_info,
			.enabledLayerCount = static_cast<uint32_t>(required_layers.size()),
			.ppEnabledLayerNames = required_layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
			.ppEnabledExtensionNames = required_extensions.data()
		};

		// Try creating the vulkan instance (will crash if not able to)
		try {
			m_instance = vk::raii::Instance(m_context, create_info);
		} catch (const vk::SystemError& e) {
			std::println(stderr, "Falied to initialize Vulkan: {}", e.what());
			std::exit(EXIT_FAILURE);
		}

		std::println("Created Vulkan Instance");
}

std::vector<const char*> Instance::GetRequiredExtensions() {
	// Getting extensions from GLFW
	uint32_t glfw_extension_count = 0;
	auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	std::println("Received {} extensions from GLFW:", glfw_extension_count);

	// Check if the glfw extensions are supported by Vulkan
	auto extension_properties = m_context.enumerateInstanceExtensionProperties();
	for (uint32_t i = 0; i < glfw_extension_count; i++) {
		auto glfw_ext = glfw_extensions[i];
		std::println("\t{}", glfw_ext);
		if (std::ranges::none_of(extension_properties, [glfw_ext](auto const& vk_ext) {
			return strcmp(vk_ext.extensionName, glfw_ext) == 0;
		})) {
			throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfw_ext));
		}
	}

	// Add the Validation layers extension if we're using validation layers
	std::vector<const char*> extensions {glfw_extensions, glfw_extensions + glfw_extension_count};
	if constexpr (VALIDATION_LAYERS_ENABLED) {
		extensions.emplace_back(vk::EXTDebugUtilsExtensionName);
		std::println("\t{}", vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

void Instance::SetupDebugMessenger() {
	if constexpr (VALIDATION_LAYERS_ENABLED) return;

	using severity_t = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using message_t = vk::DebugUtilsMessageTypeFlagBitsEXT;

	// Select which messages will be displayed
	// By severity
	vk::DebugUtilsMessageSeverityFlagsEXT severity_flags {
		severity_t::eVerbose | // Diagnostics
		severity_t::eInfo |    // Creation of resources
		severity_t::eWarning | // Unintended behaviour
		severity_t::eError     // Invalid/crash-prone
	};

	// By type
	vk::DebugUtilsMessageTypeFlagsEXT type_flags {
		message_t::eGeneral |     // General events
		message_t::ePerformance | // Non-optimal use of Vulkan
		message_t::eValidation    // Validation layers violation
	};

	vk::DebugUtilsMessengerCreateInfoEXT messenger_info {
		.messageSeverity = severity_flags,
		.messageType = type_flags,
		.pfnUserCallback = &DebugCallback
	};
	m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(messenger_info);
}

static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT type,
	const vk::DebugUtilsMessengerCallbackDataEXT* callback_data,
	void*
) {
	using severity_t = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using message_t = vk::DebugUtilsMessageTypeFlagBitsEXT;

	// Convert type to string
	std::string type_str;
	if (type == message_t::ePerformance) type_str = "Performance";
	else if (type == message_t::eValidation) type_str = "Validation Layer";
	else type_str = "General";

	// Dispatch logs
	switch (severity) {
		case severity_t::eVerbose:
			std::print("({0}) {1}", type_str, callback_data->pMessage);
			break;
		case severity_t::eInfo:
			std::print("({0}) {1}", type_str, callback_data->pMessage);
			break;
		case severity_t::eWarning:
			std::print("({0}) {1}", type_str, callback_data->pMessage);
			break;
		case severity_t::eError:
			std::print(stderr, "({0}) {1}", type_str, callback_data->pMessage);
			break;
	}

	return vk::False;
}

}

