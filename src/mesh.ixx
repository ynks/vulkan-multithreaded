/// @file mesh.ixx
/// @author Xein
/// @date 28-Nov-2025

module;

#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include <glm/glm.hpp>

export module vulkan.mesh;
import vulkan.buffers;

namespace vulkan {

export struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 color;

	static vk::VertexInputBindingDescription GetBindingDescription() {
		return vk::VertexInputBindingDescription{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = vk::VertexInputRate::eVertex
		};
	}

	static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions() {
		return {
			// Position
			vk::VertexInputAttributeDescription{
				.location = 0,
				.binding = 0,
				.format = vk::Format::eR32G32B32Sfloat,
				.offset = offsetof(Vertex, position)
			},
			// Normal
			vk::VertexInputAttributeDescription{
				.location = 1,
				.binding = 0,
				.format = vk::Format::eR32G32B32Sfloat,
				.offset = offsetof(Vertex, normal)
			},
			// TexCoord
			vk::VertexInputAttributeDescription{
				.location = 2,
				.binding = 0,
				.format = vk::Format::eR32G32Sfloat,
				.offset = offsetof(Vertex, texCoord)
			},
			// Color
			vk::VertexInputAttributeDescription{
				.location = 3,
				.binding = 0,
				.format = vk::Format::eR32G32B32Sfloat,
				.offset = offsetof(Vertex, color)
			}
		};
	}
};

export class Mesh {
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	/// @brief Bind this mesh's buffers to a command buffer
	void Bind(vk::raii::CommandBuffer& cmdBuffer) const;

	/// @brief Draw this mesh
	void Draw(vk::raii::CommandBuffer& cmdBuffer, uint32_t instanceCount = 1) const;

	/// @brief Bind and draw in one call
	void BindAndDraw(vk::raii::CommandBuffer& cmdBuffer, uint32_t instanceCount = 1) const {
		Bind(cmdBuffer);
		Draw(cmdBuffer, instanceCount);
	}

	[[nodiscard]]
	uint32_t GetVertexCount() const { return m_vertexCount; }

	[[nodiscard]]
	uint32_t GetIndexCount() const { return m_indexCount; }

	[[nodiscard]]
	bool IsIndexed() const { return m_indexCount > 0; }

	/// @brief Create a simple triangle mesh
	static Mesh CreateTriangle();

	/// @brief Create a quad mesh
	static Mesh CreateQuad();

	/// @brief Create a cube mesh
	static Mesh CreateCube();

private:
	std::unique_ptr<Buffer> m_vertexBuffer;
	std::unique_ptr<Buffer> m_indexBuffer;
	uint32_t m_vertexCount;
	uint32_t m_indexCount;
};

}
