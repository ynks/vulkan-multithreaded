/// @file command_pool.cpp
/// @author Xein
/// @date 28-Nov-2025
module;

#include <print>
#include <vulkan/vulkan_raii.hpp>
#include <thread>
#include <memory>

module vulkan.commandpool;
import vulkan.device;
import vulkan.commandbuffer;

namespace vulkan {

CommandPool::CommandPool() {
	std::println("Creating Command Pool for thread {}...", std::this_thread::get_id());

	vk::CommandPoolCreateInfo pool_info = {
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = Device::graphicsIndex()
	};
	m_commandPool = { Device::get(), pool_info };
}

CommandPool& CommandPool::GetForCurrentThread() {
	thread_local std::unique_ptr<CommandPool> t_commandPool;
	
	if (!t_commandPool) {
		t_commandPool = std::make_unique<CommandPool>();
	}
	
	return *t_commandPool;
}

CommandBuffer CommandPool::AllocateBuffer(vk::CommandBufferLevel level) {
	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = m_commandPool,
		.level = level,
		.commandBufferCount = 1
	};
	
	auto buffers = vk::raii::CommandBuffers(Device::get(), allocInfo);
	return CommandBuffer(std::move(buffers.front()));
}

std::vector<CommandBuffer> CommandPool::AllocateBuffers(uint32_t count, vk::CommandBufferLevel level) {
	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = m_commandPool,
		.level = level,
		.commandBufferCount = count
	};
	
	auto raiiBuffers = vk::raii::CommandBuffers(Device::get(), allocInfo);
	std::vector<CommandBuffer> buffers;
	buffers.reserve(count);
	
	for (auto& buffer : raiiBuffers) {
		buffers.push_back(CommandBuffer(std::move(buffer)));
	}
	
	return buffers;
}

}
