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

namespace vulkan
{
    
    
vk::VertexInputBindingDescription Vertex::getBindingDescription()
{
    return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
}

std::array<vk::VertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions()
{
    return {
        vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos) ),
        vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color) )
    };
}

Buffer::Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::BufferCreateInfo bufferInfo{ .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive };
    m_buffer = vk::raii::Buffer(*Device::get(), bufferInfo);
    vk::MemoryRequirements memRequirements = m_buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties) };
    m_bufferMemory = vk::raii::DeviceMemory(*Device::get(), allocInfo);
    m_buffer.bindMemory(*m_bufferMemory, 0);
    
}

void Buffer::copyBuffer(vk::raii::Buffer& dst, vk::DeviceSize size)
{
    vk::CommandBufferAllocateInfo allocInfo{ .commandPool = *vulkan::CommandPool().get(), .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
    vk::raii::CommandBuffer commandCopyBuffer = std::move(Device::get()->allocateCommandBuffers(allocInfo).front());
    
    commandCopyBuffer.begin(vk::CommandBufferBeginInfo { .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    commandCopyBuffer.copyBuffer(m_buffer, dst, vk::BufferCopy(0, 0, size));
    commandCopyBuffer.end();
    
    vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer };
    Device::queue()->submit(submitInfo, nullptr);
    Device::queue()->waitIdle();
    
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = Device::physicalDevice()->getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices)
    : Buffer(sizeof(vertices[0]) * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
{
    std::println("Creating vertex buffer...");

    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    void* data = m_bufferMemory.mapMemory(0, bufferSize);
    std::memcpy(data, vertices.data(), bufferSize);
    m_bufferMemory.unmapMemory();


    vulkan::Buffer stagingBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    
    void* dataStaging = stagingBuffer.getMemory()->mapMemory(0, bufferSize);
    memcpy(dataStaging, vertices.data(), bufferSize);
    stagingBuffer.getMemory()->unmapMemory();

    vk::BufferCreateInfo bufferInfo{ .size = bufferSize,  .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive };
    
    // the vertex buffer is the destination of a transfer from the staging buffer
    stagingBuffer.copyBuffer(m_buffer, bufferSize);
    
}

    
}
