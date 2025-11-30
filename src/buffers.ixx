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

}
