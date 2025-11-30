/// @file mesh.cpp
/// @author Xein
/// @date 28-Nov-2025

module;

#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include <glm/glm.hpp>

module vulkan.mesh;
import vulkan.buffers;
import vulkan.device;
import vulkan.swapchain;

namespace vulkan {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	: m_vertexCount(static_cast<uint32_t>(vertices.size()))
	, m_indexCount(static_cast<uint32_t>(indices.size()))
{
	// Get the number of frames from swapchain images
	uint32_t frameCount = static_cast<uint32_t>(Swapchain::get().getImages().size());

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

	// Descriptors are initialized later via InitDescriptors with pipeline set layout
}

void Mesh::InitDescriptors(const vk::raii::DescriptorSetLayout& setLayout, uint32_t frameCount) {
	// copy layout handle
	m_descriptorSetLayout = vk::raii::DescriptorSetLayout(Device::get(), *setLayout);
	CreateUniformBuffers(frameCount);
	CreateDescriptorPool(frameCount);
	CreateDescriptorSets();
}

void Mesh::Bind(vk::raii::CommandBuffer& cmdBuffer, const vk::raii::PipelineLayout& pipelineLayout, uint32_t frameIndex) const {
	vk::Buffer vertexBuffer = *m_vertexBuffer->getBuffer();
	vk::DeviceSize offset = 0;
	cmdBuffer.bindVertexBuffers(0, vertexBuffer, offset);

	if (m_indexBuffer) {
		cmdBuffer.bindIndexBuffer(*m_indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
	}

	// Bind descriptor set for this frame
	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		*pipelineLayout,
		0,
		*m_descriptorSets[frameIndex],
		nullptr
	);
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

void Mesh::CreateUniformBuffers(uint32_t frameCount) {
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(frameCount);
	m_uniformBuffersMapped.resize(frameCount);

	for (uint32_t i = 0; i < frameCount; ++i) {
		m_uniformBuffers[i] = std::make_unique<Buffer>(
			bufferSize,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		// Map the buffer memory persistently
		m_uniformBuffersMapped[i] = m_uniformBuffers[i]->getMemory().mapMemory(0, bufferSize);
	}
}

void Mesh::CreateDescriptorPool(uint32_t frameCount) {
	vk::DescriptorPoolSize poolSize{
		.type = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = frameCount
	};

	vk::DescriptorPoolCreateInfo poolInfo{
		.flags = {},
		.maxSets = frameCount,
		.poolSizeCount = 1,
		.pPoolSizes = &poolSize
	};

	m_descriptorPool = vk::raii::DescriptorPool(Device::get(), poolInfo);
}

void Mesh::CreateDescriptorSets() {
	uint32_t frameCount = static_cast<uint32_t>(m_uniformBuffers.size());
	std::vector<vk::DescriptorSetLayout> layouts(frameCount, *m_descriptorSetLayout);

	vk::DescriptorSetAllocateInfo allocInfo{
		.descriptorPool = *m_descriptorPool,
		.descriptorSetCount = frameCount,
		.pSetLayouts = layouts.data()
	};

	m_descriptorSets = vk::raii::DescriptorSets(Device::get(), allocInfo);

	// Update descriptor sets to point to uniform buffers
	for (uint32_t i = 0; i < frameCount; ++i) {
		vk::DescriptorBufferInfo bufferInfo{
			.buffer = *m_uniformBuffers[i]->getBuffer(),
			.offset = 0,
			.range = sizeof(UniformBufferObject)
		};

		vk::WriteDescriptorSet descriptorWrite{
			.dstSet = *m_descriptorSets[i],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pImageInfo = nullptr,
			.pBufferInfo = &bufferInfo,
			.pTexelBufferView = nullptr
		};

		Device::get().updateDescriptorSets(descriptorWrite, nullptr);
	}
}

void Mesh::UpdateUniformBuffer(uint32_t frameIndex, const UniformBufferObject& ubo) {
	memcpy(m_uniformBuffersMapped[frameIndex], &ubo, sizeof(UniformBufferObject));
}

}
