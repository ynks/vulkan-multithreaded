#include <memory>
#include <print>
#include <cstdlib>
#include <vulkan/vulkan_raii.hpp>

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

	std::vector<vk::raii::Semaphore> m_presentCompleteSemaphores;
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
	std::vector<vk::raii::Fence> m_drawFences;
	uint32_t m_currentFrame = 0;

	bool m_framebufferResized = false;

	void initVulkan() {
		m_window = std::make_unique<Window>();
		m_window->setupResizeCallback(this, [](HelloTriangleApplication* app, int, int) {
			app->m_framebufferResized = true;
		});
		m_instance = std::make_unique<vulkan::Instance>();
		m_window->CreateSurface();
		m_device = std::make_unique<vulkan::Device>();
		m_swapchain = std::make_unique<vulkan::Swapchain>();
		m_pipeline = std::make_unique<vulkan::Pipeline>();
		m_commandPool = std::make_unique<vulkan::CommandPool>();
		CreateSyncObjects();
		m_commandPool->CreateCommandBuffers(static_cast<uint32_t>(m_swapchain->get()->getImages().size()));
	}

	void CreateSyncObjects() {
		uint32_t imageCount = static_cast<uint32_t>(m_swapchain->get()->getImages().size());
		m_presentCompleteSemaphores.reserve(imageCount);
		m_renderFinishedSemaphores.reserve(imageCount);
		m_drawFences.reserve(imageCount);
		
		for (uint32_t i = 0; i < imageCount; ++i) {
			m_presentCompleteSemaphores.emplace_back(*m_device->get(), vk::SemaphoreCreateInfo());
			m_renderFinishedSemaphores.emplace_back(*m_device->get(), vk::SemaphoreCreateInfo());
			m_drawFences.emplace_back(*m_device->get(), vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
		}
	}

	void drawFrame() {
		[[maybe_unused]] auto waitResult = m_device->get()->waitForFences(*m_drawFences[m_currentFrame], vk::True, std::numeric_limits<uint64_t>::max());
		
		auto [result, image_index] = m_swapchain->get()->acquireNextImage(std::numeric_limits<uint64_t>::max(), *m_presentCompleteSemaphores[m_currentFrame], nullptr);

		if (result == vk::Result::eErrorOutOfDateKHR) {
			recreateSwapChain();
			return;
		}
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		m_device->get()->resetFences(*m_drawFences[m_currentFrame]);

		m_commandPool->RecordCommandBuffer(m_currentFrame, image_index, m_pipeline.get());

		vk::PipelineStageFlags wait_destination_stage_mask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
		vk::Semaphore wait_semaphore = *m_presentCompleteSemaphores[m_currentFrame];
		vk::Semaphore signal_semaphore = *m_renderFinishedSemaphores[m_currentFrame];
		vk::CommandBuffer cmd_buffer = **m_commandPool->buffer(m_currentFrame);
		const vk::SubmitInfo submit_info{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &wait_semaphore,
			.pWaitDstStageMask = &wait_destination_stage_mask,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd_buffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &signal_semaphore
		};
		m_device->queue()->submit(submit_info, *m_drawFences[m_currentFrame]);

		vk::Semaphore present_wait_semaphore = *m_renderFinishedSemaphores[m_currentFrame];
		vk::SwapchainKHR swapchain = **m_swapchain->get();
		const vk::PresentInfoKHR present_info_KHR{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &present_wait_semaphore,
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &image_index
		};
		result = m_device->queue()->presentKHR(present_info_KHR);
		
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_framebufferResized) {
			m_framebufferResized = false;
			recreateSwapChain();
		} else if (result != vk::Result::eSuccess) {
			throw std::runtime_error("failed to present swap chain image!");
		}
		
		m_currentFrame = (m_currentFrame + 1) % m_presentCompleteSemaphores.size();
	}

	void recreateSwapChain() {
		m_swapchain->recreate();
		m_commandPool->CreateCommandBuffers(static_cast<uint32_t>(m_swapchain->get()->getImages().size()));
	}

	void mainLoop() {
		while (!m_window->shouldClose()) {
			m_window->pollEvents();
			drawFrame();
		}

		m_device->get()->waitIdle();
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
