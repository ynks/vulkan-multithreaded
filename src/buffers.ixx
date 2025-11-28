/// @file vertex.ixx
/// @author dario
/// @date 28/11/2025.
/// 
module;

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

export module vulkan.buffers;

namespace vulkan
{
export struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription();

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
};
    
export class Buffer
{
public:
    Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    ~Buffer() = default;

    void copyBuffer(vk::raii::Buffer& dst, vk::DeviceSize size);
    
    [[nodiscard]]
    vk::raii::Buffer* getBuffer() { return &m_buffer; }
    [[nodiscard]]
    vk::raii::DeviceMemory* getMemory() { return &m_bufferMemory; }
protected:
    vk::raii::Buffer m_buffer = nullptr;
    vk::raii::DeviceMemory m_bufferMemory = nullptr;
    
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    
};

export class VertexBuffer : public Buffer
{
public:
    explicit VertexBuffer(const std::vector<Vertex>& vertices);


private:
    
};
}