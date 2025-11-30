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
	Pipeline(const vk::raii::PipelineLayout& pipelineLayout);
	
	[[nodiscard]]
	vk::raii::Pipeline& get() { return m_pipeline; }
	[[nodiscard]]
	const vk::raii::PipelineLayout& GetPipelineLayout() const { return m_pipelineLayout; }
	[[nodiscard]]
	const vk::raii::DescriptorSetLayout& GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

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

	void CreateDescriptorSetLayout();
	void CreatePipelineLayout();

	void CreateGraphicsPipeline(const ShaderStages& shader_stages, const DynamicStates& dynamic_states,
	                           const vk::PipelineColorBlendStateCreateInfo& color_blending,
	                           const vk::raii::PipelineLayout* externalLayout = nullptr);

	vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr;
	vk::raii::PipelineLayout m_pipelineLayout = nullptr;
	vk::raii::Pipeline m_pipeline = nullptr;
};

}
