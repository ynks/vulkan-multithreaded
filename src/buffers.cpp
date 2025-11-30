/// @file vertex.cpp
/// @author dario
/// @date 28/11/2025.
///
module;

#include <compare>
#include <cstddef>
#include <print>
#include <vulkan/vulkan_raii.hpp>

module vulkan.buffers;
import vulkan.device;
import vulkan.commandpool;
import vulkan.commandbuffer;

namespace vulkan
{

Buffer::Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::BufferCreateInfo bufferInfo{ .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive };
    m_buffer = vk::raii::Buffer(Device::get(), bufferInfo);
    vk::MemoryRequirements memRequirements = m_buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties) };
    m_bufferMemory = vk::raii::DeviceMemory(Device::get(), allocInfo);
    m_buffer.bindMemory(*m_bufferMemory, 0);
    
}

void Buffer::copyBuffer(vk::raii::Buffer& dst, vk::DeviceSize size)
{
    CommandBuffer::ExecuteImmediate(
        vulkan::CommandPool::GetForCurrentThread().get(),
        [&](vk::raii::CommandBuffer& cmd) {
            cmd.copyBuffer(m_buffer, dst, vk::BufferCopy(0, 0, size));
        }
    );
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = Device::physicalDevice().getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
    
}
