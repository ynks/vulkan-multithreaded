/// @file instance.ixx
/// @author Xein
/// @date 26-Nov-2025

module;
#include <vulkan/vulkan_raii.hpp>

export module vulkan.instance;

namespace vulkan {

export class Instance {
public:
	Instance();
	static Instance* instance();

	[[nodiscard]] /// @brief Exposes vk::Instance
	static vk::raii::Instance* get() { return &instance()->m_instance; }

private:
	void CreateInstance();
	void SetupDebugMessenger();
	std::vector<const char*> GetRequiredExtensions();

	static Instance* m_thisInstance;
	vk::raii::Context m_context;
	vk::raii::Instance m_instance = nullptr;
	vk::raii::DebugUtilsMessengerEXT m_debugMessenger = nullptr;
};

/// @brief Public wrapper to get the vulkan instance
export Instance* instance() { return Instance::instance(); }

Instance* Instance::m_thisInstance = nullptr;

}

