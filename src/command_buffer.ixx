/// @file command_buffer.ixx
/// @author Xein
/// @date 28-Nov-2025

module;

#include <vulkan/vulkan_raii.hpp>
#include <functional>

export module vulkan.commandbuffer;
import vulkan.device;
import vulkan.swapchain;

namespace vulkan {

export class CommandBuffer {
public:
	explicit CommandBuffer(vk::raii::CommandBuffer&& buffer) 
		: m_buffer(std::move(buffer)) {}

	/// @brief Execute a recording function with automatic begin/end
	template<typename Func>
	void Record(Func&& recordFunc, vk::CommandBufferUsageFlags flags = {}, const vk::CommandBufferInheritanceInfo* inheritanceInfo = nullptr) {
		m_buffer.reset();
		m_buffer.begin(vk::CommandBufferBeginInfo{ 
			.flags = flags,
			.pInheritanceInfo = inheritanceInfo
		});
		recordFunc(m_buffer);
		m_buffer.end();
	}

	/// @brief Execute a one-time submit command (for transfers, etc.)
	template<typename Func>
	static void ExecuteImmediate(vk::raii::CommandPool& pool, Func&& func) {
		vk::CommandBufferAllocateInfo allocInfo{
			.commandPool = pool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
		};
		
		auto cmdBuffer = std::move(Device::get().allocateCommandBuffers(allocInfo).front());
		cmdBuffer.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
		func(cmdBuffer);
		cmdBuffer.end();
		
		vk::SubmitInfo submitInfo{
			.commandBufferCount = 1,
			.pCommandBuffers = &*cmdBuffer
		};
		Device::queue().submit(submitInfo, nullptr);
		Device::queue().waitIdle();
	}

	/// @brief Transition image layout helper
	static void TransitionImageLayout(
		vk::raii::CommandBuffer& cmdBuffer,
		vk::Image image,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask,
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask,
		vk::PipelineStageFlags2 dstStageMask
	) {
		vk::ImageMemoryBarrier2 barrier{
			.srcStageMask = srcStageMask,
			.srcAccessMask = srcAccessMask,
			.dstStageMask = dstStageMask,
			.dstAccessMask = dstAccessMask,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		
		vk::DependencyInfo dependencyInfo{
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier
		};
		cmdBuffer.pipelineBarrier2(dependencyInfo);
	}

	/// @brief Get the underlying vulkan command buffer
	[[nodiscard]]
	vk::raii::CommandBuffer& get() { return m_buffer; }

	[[nodiscard]]
	const vk::raii::CommandBuffer& get() const { return m_buffer; }

private:
	vk::raii::CommandBuffer m_buffer;
};

}
