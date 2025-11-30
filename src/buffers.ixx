/// @file vertex.ixx
/// @author dario
/// @date 28/11/2025.
module;

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

export module vulkan.buffers;

namespace vulkan {

	export class Buffer {
public:
	Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	~Buffer() = default;

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&&) = default;
	Buffer& operator=(Buffer&&) = default;
		

	void copyBuffer(vk::raii::Buffer& dst, vk::DeviceSize size);
	
	[[nodiscard]]
	vk::raii::Buffer& getBuffer() { return m_buffer; }
	[[nodiscard]]
	vk::raii::DeviceMemory& getMemory() { return m_bufferMemory; }
protected:
	vk::raii::Buffer m_buffer = nullptr;
	vk::raii::DeviceMemory m_bufferMemory = nullptr;

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
};

	// comented this out cuz ur doing it everything on mesh.cpp
// export class VertexBuffer : public Buffer
// {
// public:
//     explicit VertexBuffer(const std::vector<Vertex>& vertices);
//     
// };
// 
// export class IndexBuffer : public Buffer
// {
// public:
//     explicit IndexBuffer(const std::vector<uint16_t>& indices);
// };

    
}