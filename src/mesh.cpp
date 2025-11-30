/// @file mesh.cpp
/// @author Xein
/// @date 28-Nov-2025

module;

#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include <glm/glm.hpp>

module vulkan.mesh;
import vulkan.buffers;

namespace vulkan {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: m_vertexCount(static_cast<uint32_t>(vertices.size()))
	, m_indexCount(static_cast<uint32_t>(indices.size()))
{
	// Create vertex buffer
	vk::DeviceSize vertexBufferSize = sizeof(Vertex) * vertices.size();
	m_vertexBuffer = std::make_unique<Buffer>(
		vertexBufferSize,
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);

	// Upload vertex data via staging buffer
	Buffer stagingBuffer(
		vertexBufferSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	void* data = stagingBuffer.getMemory().mapMemory(0, vertexBufferSize);
	memcpy(data, vertices.data(), vertexBufferSize);
	stagingBuffer.getMemory().unmapMemory();

	stagingBuffer.copyBuffer(m_vertexBuffer->getBuffer(), vertexBufferSize);

	// Create index buffer if indices provided
	if (!indices.empty()) {
		vk::DeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();
		m_indexBuffer = std::make_unique<Buffer>(
			indexBufferSize,
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		Buffer indexStagingBuffer(
			indexBufferSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		data = indexStagingBuffer.getMemory().mapMemory(0, indexBufferSize);
		memcpy(data, indices.data(), indexBufferSize);
		indexStagingBuffer.getMemory().unmapMemory();

		indexStagingBuffer.copyBuffer(m_indexBuffer->getBuffer(), indexBufferSize);
	}
}

void Mesh::Bind(vk::raii::CommandBuffer& cmdBuffer) const {
	vk::Buffer vertexBuffer = *m_vertexBuffer->getBuffer();
	vk::DeviceSize offset = 0;
	cmdBuffer.bindVertexBuffers(0, vertexBuffer, offset);

	if (m_indexBuffer) {
		cmdBuffer.bindIndexBuffer(*m_indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
	}
}

void Mesh::Draw(vk::raii::CommandBuffer& cmdBuffer, uint32_t instanceCount) const {
	if (m_indexBuffer) {
		cmdBuffer.drawIndexed(m_indexCount, instanceCount, 0, 0, 0);
	} else {
		cmdBuffer.draw(m_vertexCount, instanceCount, 0, 0);
	}
}

Mesh Mesh::CreateTriangle() {
	std::vector<Vertex> vertices = {
		{ .position = {  0.0f, -0.5f, 0.0f }, .normal = { 0.0f, 0.0f, 1.0f }, .texCoord = { 0.5f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f } },
		{ .position = {  0.5f,  0.5f, 0.0f }, .normal = { 0.0f, 0.0f, 1.0f }, .texCoord = { 1.0f, 1.0f }, .color = { 0.0f, 1.0f, 0.0f } },
		{ .position = { -0.5f,  0.5f, 0.0f }, .normal = { 0.0f, 0.0f, 1.0f }, .texCoord = { 0.0f, 1.0f }, .color = { 0.0f, 0.0f, 1.0f } }
	};
	return Mesh(vertices, {});
}

Mesh Mesh::CreateQuad() {
	std::vector<Vertex> vertices = {
		{ .position = { -0.5f, -0.5f, 0.0f }, .normal = { 0.0f, 0.0f, 1.0f }, .texCoord = { 0.0f, 0.0f }, .color = { 1.0f, 1.0f, 1.0f } },
		{ .position = {  0.5f, -0.5f, 0.0f }, .normal = { 0.0f, 0.0f, 1.0f }, .texCoord = { 1.0f, 0.0f }, .color = { 1.0f, 1.0f, 1.0f } },
		{ .position = {  0.5f,  0.5f, 0.0f }, .normal = { 0.0f, 0.0f, 1.0f }, .texCoord = { 1.0f, 1.0f }, .color = { 1.0f, 1.0f, 1.0f } },
		{ .position = { -0.5f,  0.5f, 0.0f }, .normal = { 0.0f, 0.0f, 1.0f }, .texCoord = { 0.0f, 1.0f }, .color = { 1.0f, 1.0f, 1.0f } }
	};
	std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
	return Mesh(vertices, indices);
}

Mesh Mesh::CreateCube() {
	std::vector<Vertex> vertices = {
		// Front face
		{ .position = { -0.5f, -0.5f,  0.5f }, .normal = {  0.0f,  0.0f,  1.0f }, .texCoord = { 0.0f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f } },
		{ .position = {  0.5f, -0.5f,  0.5f }, .normal = {  0.0f,  0.0f,  1.0f }, .texCoord = { 1.0f, 0.0f }, .color = { 1.0f, 0.0f, 0.0f } },
		{ .position = {  0.5f,  0.5f,  0.5f }, .normal = {  0.0f,  0.0f,  1.0f }, .texCoord = { 1.0f, 1.0f }, .color = { 1.0f, 0.0f, 0.0f } },
		{ .position = { -0.5f,  0.5f,  0.5f }, .normal = {  0.0f,  0.0f,  1.0f }, .texCoord = { 0.0f, 1.0f }, .color = { 1.0f, 0.0f, 0.0f } },
		// Back face
		{ .position = {  0.5f, -0.5f, -0.5f }, .normal = {  0.0f,  0.0f, -1.0f }, .texCoord = { 0.0f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f } },
		{ .position = { -0.5f, -0.5f, -0.5f }, .normal = {  0.0f,  0.0f, -1.0f }, .texCoord = { 1.0f, 0.0f }, .color = { 0.0f, 1.0f, 0.0f } },
		{ .position = { -0.5f,  0.5f, -0.5f }, .normal = {  0.0f,  0.0f, -1.0f }, .texCoord = { 1.0f, 1.0f }, .color = { 0.0f, 1.0f, 0.0f } },
		{ .position = {  0.5f,  0.5f, -0.5f }, .normal = {  0.0f,  0.0f, -1.0f }, .texCoord = { 0.0f, 1.0f }, .color = { 0.0f, 1.0f, 0.0f } },
		// Top face
		{ .position = { -0.5f,  0.5f,  0.5f }, .normal = {  0.0f,  1.0f,  0.0f }, .texCoord = { 0.0f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f } },
		{ .position = {  0.5f,  0.5f,  0.5f }, .normal = {  0.0f,  1.0f,  0.0f }, .texCoord = { 1.0f, 0.0f }, .color = { 0.0f, 0.0f, 1.0f } },
		{ .position = {  0.5f,  0.5f, -0.5f }, .normal = {  0.0f,  1.0f,  0.0f }, .texCoord = { 1.0f, 1.0f }, .color = { 0.0f, 0.0f, 1.0f } },
		{ .position = { -0.5f,  0.5f, -0.5f }, .normal = {  0.0f,  1.0f,  0.0f }, .texCoord = { 0.0f, 1.0f }, .color = { 0.0f, 0.0f, 1.0f } },
		// Bottom face
		{ .position = { -0.5f, -0.5f, -0.5f }, .normal = {  0.0f, -1.0f,  0.0f }, .texCoord = { 0.0f, 0.0f }, .color = { 1.0f, 1.0f, 0.0f } },
		{ .position = {  0.5f, -0.5f, -0.5f }, .normal = {  0.0f, -1.0f,  0.0f }, .texCoord = { 1.0f, 0.0f }, .color = { 1.0f, 1.0f, 0.0f } },
		{ .position = {  0.5f, -0.5f,  0.5f }, .normal = {  0.0f, -1.0f,  0.0f }, .texCoord = { 1.0f, 1.0f }, .color = { 1.0f, 1.0f, 0.0f } },
		{ .position = { -0.5f, -0.5f,  0.5f }, .normal = {  0.0f, -1.0f,  0.0f }, .texCoord = { 0.0f, 1.0f }, .color = { 1.0f, 1.0f, 0.0f } },
		// Right face
		{ .position = {  0.5f, -0.5f,  0.5f }, .normal = {  1.0f,  0.0f,  0.0f }, .texCoord = { 0.0f, 0.0f }, .color = { 1.0f, 0.0f, 1.0f } },
		{ .position = {  0.5f, -0.5f, -0.5f }, .normal = {  1.0f,  0.0f,  0.0f }, .texCoord = { 1.0f, 0.0f }, .color = { 1.0f, 0.0f, 1.0f } },
		{ .position = {  0.5f,  0.5f, -0.5f }, .normal = {  1.0f,  0.0f,  0.0f }, .texCoord = { 1.0f, 1.0f }, .color = { 1.0f, 0.0f, 1.0f } },
		{ .position = {  0.5f,  0.5f,  0.5f }, .normal = {  1.0f,  0.0f,  0.0f }, .texCoord = { 0.0f, 1.0f }, .color = { 1.0f, 0.0f, 1.0f } },
		// Left face
		{ .position = { -0.5f, -0.5f, -0.5f }, .normal = { -1.0f,  0.0f,  0.0f }, .texCoord = { 0.0f, 0.0f }, .color = { 0.0f, 1.0f, 1.0f } },
		{ .position = { -0.5f, -0.5f,  0.5f }, .normal = { -1.0f,  0.0f,  0.0f }, .texCoord = { 1.0f, 0.0f }, .color = { 0.0f, 1.0f, 1.0f } },
		{ .position = { -0.5f,  0.5f,  0.5f }, .normal = { -1.0f,  0.0f,  0.0f }, .texCoord = { 1.0f, 1.0f }, .color = { 0.0f, 1.0f, 1.0f } },
		{ .position = { -0.5f,  0.5f, -0.5f }, .normal = { -1.0f,  0.0f,  0.0f }, .texCoord = { 0.0f, 1.0f }, .color = { 0.0f, 1.0f, 1.0f } },
	};

	std::vector<uint32_t> indices = {
		0,  1,  2,  2,  3,  0,  // Front
		4,  5,  6,  6,  7,  4,  // Back
		8,  9, 10, 10, 11,  8,  // Top
		12, 13, 14, 14, 15, 12, // Bottom
		16, 17, 18, 18, 19, 16, // Right
		20, 21, 22, 22, 23, 20  // Left
	};

	return Mesh(vertices, indices);
}

}
