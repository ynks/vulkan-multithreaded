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
import vulkan.commandbuffer;
import vulkan.mesh;

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
	std::unique_ptr<vulkan::Mesh> m_mesh;

	std::vector<vulkan::CommandBuffer> m_commandBuffers;
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
		CreateSyncObjects();
		CreateMesh();
		CreateCommandBuffers();
	}

	void CreateSyncObjects() {
		uint32_t imageCount = static_cast<uint32_t>(m_swapchain->get().getImages().size());
		m_presentCompleteSemaphores.reserve(imageCount);
		m_renderFinishedSemaphores.reserve(imageCount);
		m_drawFences.reserve(imageCount);
		
		for (uint32_t i = 0; i < imageCount; ++i) {
			m_presentCompleteSemaphores.emplace_back(m_device->get(), vk::SemaphoreCreateInfo());
			m_renderFinishedSemaphores.emplace_back(m_device->get(), vk::SemaphoreCreateInfo());
			m_drawFences.emplace_back(m_device->get(), vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
		}
	}

	void CreateMesh() {
		m_mesh = std::make_unique<vulkan::Mesh>(vulkan::Mesh::CreateTriangle());
	}

	void CreateCommandBuffers() {
		auto& pool = vulkan::CommandPool::GetForCurrentThread();
		uint32_t imageCount = static_cast<uint32_t>(m_swapchain->get().getImages().size());
		m_commandBuffers = pool.AllocateBuffers(imageCount);
	}

	void drawFrame() {
		[[maybe_unused]] auto waitResult = m_device->get().waitForFences(*m_drawFences[m_currentFrame], vk::True, std::numeric_limits<uint64_t>::max());
		
		auto [result, image_index] = m_swapchain->get().acquireNextImage(std::numeric_limits<uint64_t>::max(), *m_presentCompleteSemaphores[m_currentFrame], nullptr);

		if (result == vk::Result::eErrorOutOfDateKHR) {
			recreateSwapChain();
			return;
		}
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		m_device->get().resetFences(*m_drawFences[m_currentFrame]);

		// Record command buffer using lambda
		m_commandBuffers[m_currentFrame].Record([&](vk::raii::CommandBuffer& cmd) {
			// Transition image for rendering
			vulkan::CommandBuffer::TransitionImageLayout(
				cmd,
				vulkan::Swapchain::image(image_index),
				vk::ImageLayout::eUndefined,  // First frame or after present
				vk::ImageLayout::eColorAttachmentOptimal,
				{},
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eTopOfPipe,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput
			);

			// Begin rendering
			vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
			vk::RenderingAttachmentInfo colorAttachment{
				.imageView = vulkan::Swapchain::view(image_index),
				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.clearValue = clearColor
			};

			vk::RenderingInfo renderingInfo{
				.renderArea = { .offset = {0, 0}, .extent = vulkan::Swapchain::extent() },
				.layerCount = 1,
				.colorAttachmentCount = 1,
				.pColorAttachments = &colorAttachment
			};

			cmd.beginRendering(renderingInfo);

			// Bind pipeline and set dynamic state
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->get());
			auto extent = vulkan::Swapchain::extent();
			cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f));
			cmd.setScissor(0, vk::Rect2D({0, 0}, extent));

			// Draw mesh
			m_mesh->BindAndDraw(cmd);

			cmd.endRendering();

			// Transition for present
			vulkan::CommandBuffer::TransitionImageLayout(
				cmd,
				vulkan::Swapchain::image(image_index),
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::ePresentSrcKHR,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				{},
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::PipelineStageFlagBits2::eBottomOfPipe
			);
		});

		// Submit
		vk::PipelineStageFlags waitStage(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		vk::Semaphore waitSemaphore = *m_presentCompleteSemaphores[m_currentFrame];
		vk::Semaphore signalSemaphore = *m_renderFinishedSemaphores[m_currentFrame];
		vk::CommandBuffer cmdBuffer = *m_commandBuffers[m_currentFrame].get();

		vk::SubmitInfo submitInfo{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &waitSemaphore,
			.pWaitDstStageMask = &waitStage,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmdBuffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &signalSemaphore
		};
		m_device->queue().submit(submitInfo, *m_drawFences[m_currentFrame]);

		// Present
		vk::SwapchainKHR swapchain = *m_swapchain->get();
		vk::PresentInfoKHR presentInfo{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &signalSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &image_index
		};
		result = m_device->queue().presentKHR(presentInfo);
		
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
		CreateCommandBuffers();
	}

	void mainLoop() {
		while (!m_window->shouldClose()) {
			m_window->pollEvents();
			drawFrame();
		}

		m_device->get().waitIdle();
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
