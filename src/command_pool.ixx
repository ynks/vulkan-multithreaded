/// @file command_pool.ixx
/// @author Xein
/// @date 28-Nov-2025
module;

#include <vulkan/vulkan_raii.hpp>
#include <print>

export module vulkan.commandpool;
import vulkan.device;
import vulkan.swapchain;
import vulkan.pipeline;

namespace vulkan {

export class CommandPool {
public:
	CommandPool();
	void CreateCommandBuffers(uint32_t count);
	void RecordCommandBuffer(uint32_t frameIndex, uint32_t imageIndex, Pipeline* pipeline);

	vk::raii::CommandPool* get() { return &m_commandPool; };
	vk::raii::CommandBuffer* buffer(uint32_t index) { return &m_buffers[index]; };

private:
	vk::raii::CommandPool m_commandPool = nullptr;
	std::vector<vk::raii::CommandBuffer> m_buffers;

	void TransitionImageLayout (
		vk::raii::CommandBuffer& cmdBuffer,
		uint32_t imageIndex,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask,
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask,
		vk::PipelineStageFlags2 dstStageMask
	);
};

CommandPool::CommandPool() {
	std::println("Creating Command Pool...");

	vk::CommandPoolCreateInfo pool_info = {
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = Device::graphicsIndex()
	};
	m_commandPool = { *Device::get(), pool_info };
}

void CommandPool::CreateCommandBuffers(uint32_t count) {
	std::println("Creating {} Command Buffers...", count);
	vk::CommandBufferAllocateInfo buffer_info {
		.commandPool = m_commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = count
	};

	m_buffers = vk::raii::CommandBuffers(*Device::get(), buffer_info);
}

void CommandPool::RecordCommandBuffer(uint32_t frameIndex, uint32_t imageIndex, Pipeline* pipeline) {
	auto& cmdBuffer = m_buffers[frameIndex];
	cmdBuffer.reset();
	cmdBuffer.begin({});

	// Set the image format and clean the buffer
	TransitionImageLayout (
		cmdBuffer,
		imageIndex,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput
	);

	vk::ClearValue clear_color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	vk::RenderingAttachmentInfo attachment_info = {
		.imageView = Swapchain::view(imageIndex),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp= vk::AttachmentStoreOp::eStore,
		.clearValue = clear_color
	};
	vk::RenderingInfo rendering_info = {
		.renderArea = { .offset = { 0, 0 }, .extent = Swapchain::extent() },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachment_info
	};

	// actual rendering
	cmdBuffer.beginRendering(rendering_info);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->get());
	auto extent = Swapchain::extent();
	cmdBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f));
	cmdBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

	cmdBuffer.draw(3, 1, 0, 0);

	cmdBuffer.endRendering();

	// transition the image format to PRESENT_SRC
	TransitionImageLayout(
		cmdBuffer,
		imageIndex,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
		{},                                                     // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe               // dstStage
	);

	cmdBuffer.end();
}

void CommandPool::TransitionImageLayout (
	vk::raii::CommandBuffer& cmdBuffer,
	uint32_t imageIndex,
	vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout,
	vk::AccessFlags2 srcAccessMask,
	vk::AccessFlags2 dstAccessMask,
	vk::PipelineStageFlags2 srcStageMask,
	vk::PipelineStageFlags2 dstStageMask
) {
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = srcStageMask,
		.srcAccessMask = srcAccessMask,
		.dstStageMask = dstStageMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = Swapchain::image(imageIndex),
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	vk::DependencyInfo dependencyInfo = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};
	cmdBuffer.pipelineBarrier2(dependencyInfo);
}

}
