/// @file pipeline.ixx
/// @author Xein
/// @date 27-Nov-2025
module;

#include <fstream>
#include <print>
#include <vulkan/vulkan_raii.hpp>

export module vulkan.pipeline;
import vulkan.device;
namespace vulkan {

export class Pipeline {
public:
	Pipeline();

private:
	[[nodiscard]]
	vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& code) const;

	vk::raii::PipelineLayout m_pipelineLayout = nullptr;
};

std::vector<char> ReadFile(const std::string& path) {
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file");
	}

	std::vector<char> buffer(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
	file.close();

	return buffer;
}

Pipeline::Pipeline() {
	// modules
	auto shader_code = ReadFile("slang.spv");
	auto shaderModule = CreateShaderModule(shader_code);

	// stages
	vk::PipelineShaderStageCreateInfo vert_info {
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = shaderModule,
		.pName = "vertMain"
	};

	vk::PipelineShaderStageCreateInfo frag_info {
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = shaderModule,
		.pName = "fragMain"
	};

	std::array shader_stages = { vert_info, frag_info };

	// states
	std::vector dynamic_states = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo state_info {
		.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
		.pDynamicStates = dynamic_states.data()
	};

	vk::PipelineViewportStateCreateInfo viewport_info {
		.viewportCount = 1,
		.scissorCount = 1
	};

	// vertex
	// TODO: This is hardcoded in the shader rn
	vk::PipelineVertexInputStateCreateInfo vertex_info;
	vk::PipelineInputAssemblyStateCreateInfo assembly_info {
		.topology = vk::PrimitiveTopology::eTriangleList
	};

	// raster
	vk::PipelineRasterizationStateCreateInfo rasterizer {
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
		.depthBiasEnable = vk::False,
		.depthBiasSlopeFactor = 1.0f,
		.lineWidth = 1.0f
	};

	vk::PipelineColorBlendAttachmentState color_blend_attachment;
	color_blend_attachment.blendEnable = vk::True;
	color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
	color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;

	vk::PipelineColorBlendStateCreateInfo color_blending {
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment
	};

	// Create layout
	vk::PipelineLayoutCreateInfo pipeline_layout_info {
		.setLayoutCount = 0,
		.pushConstantRangeCount = 0
	};
	m_pipelineLayout = vk::raii::PipelineLayout(*Device::get(), pipeline_layout_info);
}

vk::raii::ShaderModule Pipeline::CreateShaderModule(const std::vector<char>& code) const {
	vk::ShaderModuleCreateInfo shader_info {
		.codeSize = code.size() * sizeof(char),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	vk::raii::ShaderModule shader_module{*Device::get(), shader_info};
	std::println("Created shader module");
	return shader_module;
}

}

