#include <memory>
#include <print>
#include <cstdlib>

import window;
import vulkan.instance;
import vulkan.device;
import vulkan.swapchain;
import vulkan.pipeline;
import vulkan.commandpool;

class HelloTriangleApplication {
public:
	void run() {
		initVulkan();
		mainLoop();
	}

private:
	std::unique_ptr<vulkan::Instance> m_instance;
	std::unique_ptr<Window> m_window;
	std::unique_ptr<vulkan::Device> m_device;
	std::unique_ptr<vulkan::Swapchain> m_swapchain;
	std::unique_ptr<vulkan::Pipeline> m_pipeline;
	std::unique_ptr<vulkan::CommandPool> m_commandPool;

	void initVulkan() {
		m_window = std::make_unique<Window>();
		m_instance = std::make_unique<vulkan::Instance>();
		m_window->CreateSurface();
		m_device = std::make_unique<vulkan::Device>();
		m_swapchain = std::make_unique<vulkan::Swapchain>();
		m_pipeline = std::make_unique<vulkan::Pipeline>();
		m_commandPool = std::make_unique<vulkan::CommandPool>();
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
