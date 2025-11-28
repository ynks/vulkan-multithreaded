/// @file command_pool.ixx
/// @author Xein
/// @date 28-Nov-2025
module;

#include <vulkan/vulkan_raii.hpp>

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

	[[nodiscard]]
	vk::raii::CommandPool* get() { return &m_commandPool; }
	[[nodiscard]]
	vk::raii::CommandBuffer* buffer(uint32_t index) { return &m_buffers[index]; }

private:
	vk::raii::CommandPool m_commandPool = nullptr;
	std::vector<vk::raii::CommandBuffer> m_buffers;

	void TransitionImageLayout(
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

}

