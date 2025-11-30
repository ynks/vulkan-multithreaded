/// @file command_pool.ixx
/// @author Xein
/// @date 28-Nov-2025

module;

#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include <vector>

export module vulkan.commandpool;
import vulkan.device;
import vulkan.commandbuffer;

namespace vulkan {

export class CommandPool {
public:
	CommandPool();

	static CommandPool& GetForCurrentThread();

	/// @brief Allocate a single command buffer
	[[nodiscard]]
	CommandBuffer AllocateBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

	/// @brief Allocate multiple command buffers
	[[nodiscard]]
	std::vector<CommandBuffer> AllocateBuffers(uint32_t count, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

	/// @brief Get the underlying pool
	[[nodiscard]]
	vk::raii::CommandPool& get() { return m_commandPool; }

	/// @brief Reset the entire pool
	void Reset(vk::CommandPoolResetFlags flags = {}) {
		m_commandPool.reset(flags);
	}

private:
	vk::raii::CommandPool m_commandPool = nullptr;
};

}

