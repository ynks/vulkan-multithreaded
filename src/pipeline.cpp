/// @file pipeline.cpp
/// @author Xein
/// @date 27-Nov-2025
module;

#include <fstream>
#include <print>
#include <vulkan/vulkan_raii.hpp>

module vulkan.pipeline;
import vulkan.device;
import vulkan.swapchain;

namespace vulkan {

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
	std::println("Creating pipeline...");

	auto shader_stages = CreateShaderStages();
	auto dynamic_states = CreateDynamicStates();

	vk::PipelineColorBlendAttachmentState color_blend_attachment;
	auto color_blending = CreateColorBlendState(color_blend_attachment);

	CreatePipelineLayout();
	CreateGraphicsPipeline(shader_stages, dynamic_states, color_blending);

	std::println("Created Pipeline");
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

Pipeline::ShaderStages Pipeline::CreateShaderStages() {
	auto shader_code = ReadFile("slang.spv");
	auto shaderModule = CreateShaderModule(shader_code);

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

	std::println("Created shader stages");
	return ShaderStages {
		.stages = { vert_info, frag_info },
		.module = std::move(shaderModule)
	};
}

Pipeline::DynamicStates Pipeline::CreateDynamicStates() {
	std::vector<vk::DynamicState> states = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo info {
		.dynamicStateCount = static_cast<uint32_t>(states.size()),
		.pDynamicStates = states.data()
	};

	return DynamicStates {
		.states = std::move(states),
		.info = info
	};
}

vk::PipelineColorBlendStateCreateInfo Pipeline::CreateColorBlendState(vk::PipelineColorBlendAttachmentState& attachment) {
	attachment.blendEnable = vk::True;
	attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	attachment.colorBlendOp = vk::BlendOp::eAdd;
	attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	attachment.alphaBlendOp = vk::BlendOp::eAdd;

	return vk::PipelineColorBlendStateCreateInfo {
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &attachment
	};
}

void Pipeline::CreatePipelineLayout() {
	vk::PipelineLayoutCreateInfo pipeline_layout_info {
		.setLayoutCount = 0,
		.pushConstantRangeCount = 0
	};
	m_pipelineLayout = vk::raii::PipelineLayout(*Device::get(), pipeline_layout_info);
	std::println("Created Pipeline Layout");
}

void Pipeline::CreateGraphicsPipeline(const ShaderStages& shader_stages, const DynamicStates& dynamic_states,
                                      const vk::PipelineColorBlendStateCreateInfo& color_blending) {
	vk::PipelineViewportStateCreateInfo viewport_info {
		.viewportCount = 1,
		.scissorCount = 1
	};

	vk::PipelineVertexInputStateCreateInfo vertex_info;
	vk::PipelineInputAssemblyStateCreateInfo assembly_info {
		.topology = vk::PrimitiveTopology::eTriangleList
	};

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

	vk::PipelineMultisampleStateCreateInfo multisampling {
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False
	};

	auto sw_format = Swapchain::format();
	vk::PipelineRenderingCreateInfo pipeline_rendering_info {
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &sw_format
	};

	vk::GraphicsPipelineCreateInfo pipeline_info {
		.pNext = &pipeline_rendering_info,
		.stageCount = 2,
		.pStages = shader_stages.stages.data(),
		.pVertexInputState = &vertex_info,
		.pInputAssemblyState = &assembly_info,
		.pViewportState = &viewport_info,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pColorBlendState = &color_blending,
		.pDynamicState = &dynamic_states.info,
		.layout = m_pipelineLayout,
		.renderPass = nullptr
	};

	m_pipeline = vk::raii::Pipeline(*Device::get(), nullptr, pipeline_info);
}

}
