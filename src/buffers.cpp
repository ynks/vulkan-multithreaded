/// @file vertex.cpp
/// @author dario
/// @date 28/11/2025.
///
module;

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

////////////////////////////////////////////////////////////////////////////////////
/// Vertex Buffer
////////////////////////////////////////////////////////////////////////////////////

///
///@note rn we are creating 2 buffers, one staging buffer in host visible memory and one vertex buffer in device local memory
/// we copy the data from the staging buffer to the vertex buffer
///
// VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices)
//     : Buffer(sizeof(vertices[0]) * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer,
//         vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
// {
//     std::println("Creating vertex buffer...");
// 
//     vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
// 
//     void* data = m_bufferMemory.mapMemory(0, bufferSize);
//     std::memcpy(data, vertices.data(), bufferSize);
//     m_bufferMemory.unmapMemory();
// 
//     // create a staging buffer to transfer data to the GPU
//     vulkan::Buffer stagingBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//     
//     void* dataStaging = stagingBuffer.getMemory()->mapMemory(0, bufferSize);
//     memcpy(dataStaging, vertices.data(), bufferSize);
//     stagingBuffer.getMemory()->unmapMemory();
// 
//     vk::BufferCreateInfo bufferInfo{ .size = bufferSize,  .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive };
// 
//     // the vertex buffer is the destination of a transfer from the staging buffer
//     stagingBuffer.copyBuffer(m_buffer, bufferSize);
// 
// }
// 
//     ////////////////////////////////////////////////////////////////////////////////////
//     /// Index Buffer
//     ////////////////////////////////////////////////////////////////////////////////////
// IndexBuffer::IndexBuffer(const std::vector<uint16_t>& indices)
//     : Buffer(sizeof(indices[0]) * indices.size(), vk::BufferUsageFlagBits::eIndexBuffer,
//         vk::MemoryPropertyFlagBits::eDeviceLocal)
// {
//     vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();
// 
//     Buffer stagingBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
// 
//     void* data = stagingBuffer.getMemory()->mapMemory(0, bufferSize);
//     memcpy(data, indices.data(), (size_t) bufferSize);
//     stagingBuffer.getMemory()->unmapMemory();
//     
//     stagingBuffer.copyBuffer(m_buffer, bufferSize);
// }
}
