#include <vulkan/vulkan_raii.hpp>
#include <print>
#include <GLFW/glfw3.h>

import vulkan.instance;

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
	std::unique_ptr<vulkan::Instance> m_instance;

	void initWindow() {
		std::println("Initializing GLFW");
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_NAME, nullptr, nullptr);
		std::println("Created window \"{}\"", WINDOW_NAME);
	}


	void initVulkan() {
		m_instance = std::make_unique<vulkan::Instance>();
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
