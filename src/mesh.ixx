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

export struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

export class Mesh {
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	// New: create mesh UBO/descriptors from an external descriptor set layout
	void InitDescriptors(const vk::raii::DescriptorSetLayout& setLayout, uint32_t frameCount);

	/// @brief Bind this mesh's buffers to a command buffer
	void Bind(vk::raii::CommandBuffer& cmdBuffer, const vk::raii::PipelineLayout& pipelineLayout, uint32_t frameIndex) const;

	/// @brief Draw this mesh
	void Draw(vk::raii::CommandBuffer& cmdBuffer, uint32_t instanceCount = 1) const;

	/// @brief Bind and draw in one call
	void BindAndDraw(vk::raii::CommandBuffer& cmdBuffer, const vk::raii::PipelineLayout& pipelineLayout, uint32_t frameIndex, uint32_t instanceCount = 1) const {
		Bind(cmdBuffer, pipelineLayout, frameIndex);
		Draw(cmdBuffer, instanceCount);
	}

	void UpdateUniformBuffer(uint32_t frameIndex, const UniformBufferObject& ubo);

	[[nodiscard]]
	uint32_t GetVertexCount() const { return m_vertexCount; }
	[[nodiscard]]
	uint32_t GetIndexCount() const { return m_indexCount; }
	[[nodiscard]]
	bool IsIndexed() const { return m_indexCount > 0; }

	static Mesh CreateTriangle();
	static Mesh CreateQuad();
	static Mesh CreateCube();

private:
	void CreateUniformBuffers(uint32_t frameCount);
	void CreateDescriptorPool(uint32_t frameCount);
	void CreateDescriptorSets();

	std::unique_ptr<Buffer> m_vertexBuffer;
	std::unique_ptr<Buffer> m_indexBuffer;

	std::vector<std::unique_ptr<Buffer>> m_uniformBuffers;
	std::vector<void*> m_uniformBuffersMapped;

	vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr; // external-owned copy
	vk::raii::DescriptorPool m_descriptorPool = nullptr;
	vk::raii::DescriptorSets m_descriptorSets = nullptr;
	
	uint32_t m_vertexCount;
	uint32_t m_indexCount;
};

}
