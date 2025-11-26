#include <vulkan/vulkan_raii.hpp>
#include <print>
#include <GLFW/glfw3.h>

constexpr uint32_t WIDTH  = 800;
constexpr uint32_t HEIGHT = 600;
constexpr const char* WINDOW_NAME = "CS180 final";

class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow *window = nullptr;

	// vk instance
	vk::raii::Context m_context;
	vk::raii::Instance m_instance = nullptr;

	void initWindow() {
		std::print("Initializing GLFW");
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_NAME, nullptr, nullptr);
		std::print("Created window \"{}\"", WINDOW_NAME);
	}

	void createInstance() {
		std::print("Started Vulkan instance creation...");

		// Contains information about the application
		constexpr vk::ApplicationInfo app_info = {
			.pApplicationName = WINDOW_NAME,
			.applicationVersion = VK_MAKE_VERSION(1, 0 ,0),
			.pEngineName = "Toast Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
				.apiVersion = vk::ApiVersion14
		};

		// Getting extensions from GLFW
		uint32_t glfw_extension_count = 0;
		auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		std::print("Received {} extensions from GLFW:", glfw_extension_count);

		// Check if the glfw extensions are supported by Vulkan
		auto extension_properties = m_context.enumerateInstanceExtensionProperties();
		for (uint32_t i = 0; i < glfw_extension_count; i++) {
			auto glfw_ext = glfw_extensions[i];
			std::print("\t{}", glfw_ext);
			if (std::ranges::none_of(extension_properties, [glfw_ext](auto const& vk_ext) { \
				return strcmp(vk_ext.extensionName, glfw_ext) == 0;
			})) {
				throw std::runtime_error("Required GLFW extension not supported" + std::string(glfw_ext));
			}
		}

		// Contains information about the vulkan instance
		vk::InstanceCreateInfo create_info {
			.pApplicationInfo = &app_info,
			.enabledExtensionCount = glfw_extension_count,
			.ppEnabledExtensionNames = glfw_extensions
		};

		// Try creating the vulkan instance (will crash if not able to)
		try {
			m_instance = vk::raii::Instance(m_context, create_info);
		} catch (const vk::SystemError& e) {
			std::print(stderr, "Falied to initialize Vulkan: {}", e.what());
			std::exit(EXIT_FAILURE);
		}

		std::print("Created Vulkan Instance");
	}

	void initVulkan() {
		createInstance();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		glfwDestroyWindow(window);

		glfwTerminate();
	}
};


int main() {
	try {
		HelloTriangleApplication app;
		app.run();
	} catch (const std::exception &e) {
		std::println(stderr, "{}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
