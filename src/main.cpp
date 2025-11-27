#include <memory>
#include <print>
#include <cstdlib>

import window;
import vulkan.instance;
import vulkan.device;

class HelloTriangleApplication {
public:
	void run() {
		initVulkan();
		mainLoop();
	}

private:
	std::unique_ptr<Window> m_window;
	std::unique_ptr<vulkan::Instance> m_instance;
	std::unique_ptr<vulkan::Device> m_device;

	void initVulkan() {
		m_window = std::make_unique<Window>();
		m_instance = std::make_unique<vulkan::Instance>();
		m_window->CreateSurface();
		m_device = std::make_unique<vulkan::Device>();
	}

	void mainLoop() {
		while (!m_window->shouldClose()) {
			m_window->pollEvents();
		}
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
