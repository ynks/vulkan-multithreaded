/// @file pipeline.ixx
/// @author Xein
/// @date 27-Nov-2025
module;

#include <array>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

export module vulkan.pipeline;
import vulkan.device;
import vulkan.swapchain;

namespace vulkan {

export class Pipeline {
public:
	Pipeline();

private:
	struct ShaderStages {
		std::array<vk::PipelineShaderStageCreateInfo, 2> stages;
		vk::raii::ShaderModule module;
	};

	struct DynamicStates {
		std::vector<vk::DynamicState> states;
		vk::PipelineDynamicStateCreateInfo info;
	};

	[[nodiscard]]
	vk::raii::ShaderModule CreateShaderModule(const std::vector<char>& code) const;

	[[nodiscard]]
	ShaderStages CreateShaderStages();

	[[nodiscard]]
	DynamicStates CreateDynamicStates();

	[[nodiscard]]
	vk::PipelineColorBlendStateCreateInfo CreateColorBlendState(vk::PipelineColorBlendAttachmentState& attachment);

	void CreatePipelineLayout();

	void CreateGraphicsPipeline(const ShaderStages& shader_stages, const DynamicStates& dynamic_states,
	                           const vk::PipelineColorBlendStateCreateInfo& color_blending);

	vk::raii::PipelineLayout m_pipelineLayout = nullptr;
	vk::raii::Pipeline m_pipeline = nullptr;
};

}
