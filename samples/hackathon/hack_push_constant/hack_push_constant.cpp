/* Copyright (c) 2024, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hack_push_constant.h"

#include "benchmark_mode/benchmark_mode.h"

hack_push_constant::hack_push_constant()
{
	set_shading_language(vkb::ShadingLanguage::GLSL);
	title = "Hack: Constant push";
}

hack_push_constant::~hack_push_constant()
{
	if (has_device())
	{
		// Clean up used Vulkan resources
		// Note : Inherited destructor cleans up resources stored in base class
		vkDestroyPipeline(get_device().get_handle(), pipeline, nullptr);

		vkDestroyPipelineLayout(get_device().get_handle(), pipeline_layout, nullptr);
		vkDestroyDescriptorSetLayout(get_device().get_handle(), descriptor_set_layout, nullptr);
	}
}

void hack_push_constant::draw(VkCommandBuffer &commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertex_buffer->get(), offsets);
	vkCmdBindIndexBuffer(commandBuffer, index_buffer->get_handle(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

	// Render multiple objects using different model matrices by dynamically offsetting into one uniform buffer
	for (size_t j = 0; j < OBJECT_INSTANCES; j++)
	{
		vkCmdPushConstants(commandBuffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, alignment, get_aligned_model(j));
		vkCmdDrawIndexed(commandBuffer, index_count, 1, 0, 0, 0);
	}
}

void hack_push_constant::setup_descriptor_pool()
{
	// Example uses one ubo and one image sampler
	std::vector<VkDescriptorPoolSize> pool_sizes =
	{
	        vkb::initializers::descriptor_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)};

	VkDescriptorPoolCreateInfo descriptor_pool_create_info =
		vkb::initializers::descriptor_pool_create_info(
			static_cast<uint32_t>(pool_sizes.size()),
			pool_sizes.data(),
	        1);

	VK_CHECK(vkCreateDescriptorPool(get_device().get_handle(), &descriptor_pool_create_info, nullptr, &descriptor_pool));
}

void hack_push_constant::setup_descriptor_set_layout()
{
	std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings =
	{
			vkb::initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
	};

	VkDescriptorSetLayoutCreateInfo descriptor_layout =
		vkb::initializers::descriptor_set_layout_create_info(
			set_layout_bindings.data(),
			static_cast<uint32_t>(set_layout_bindings.size()));

	VK_CHECK(vkCreateDescriptorSetLayout(get_device().get_handle(), &descriptor_layout, nullptr, &descriptor_set_layout));

	range            = {};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	range.offset     = 0;
	range.size       = alignment;

	VkPipelineLayoutCreateInfo pipeline_layout_create_info =
		vkb::initializers::pipeline_layout_create_info(
			&descriptor_set_layout,
			1);

	pipeline_layout_create_info.pushConstantRangeCount = 1;
	pipeline_layout_create_info.pPushConstantRanges    = &range;

	VK_CHECK(vkCreatePipelineLayout(get_device().get_handle(), &pipeline_layout_create_info, nullptr, &pipeline_layout));
}

void hack_push_constant::setup_descriptor_set()
{
	VkDescriptorSetAllocateInfo alloc_info =
	    vkb::initializers::descriptor_set_allocate_info(
	        descriptor_pool,
	        &descriptor_set_layout,
	        1);

	
	VK_CHECK(vkAllocateDescriptorSets(get_device().get_handle(), &alloc_info, &descriptor_set));
	VkDescriptorBufferInfo view_buffer_descriptor = create_descriptor(*view_uniform_buffer.view);

	std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
	    // Binding 0 : Projection/View matrix uniform buffer
	    vkb::initializers::write_descriptor_set(descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &view_buffer_descriptor),
	};

	vkUpdateDescriptorSets(get_device().get_handle(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, NULL);

}

void hack_push_constant::prepare_pipelines()
{
	// Load shaders
	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;

	shader_stages[0] = load_shader("hackathon", "push_constant.vert", VK_SHADER_STAGE_VERTEX_BIT);
	shader_stages[1] = load_shader("hackathon", "base.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

	VkGraphicsPipelineCreateInfo pipeline_create_info =
		vkb::initializers::pipeline_create_info(
			pipeline_layout,
			render_pass,
			0);

	pipeline_create_info.pVertexInputState = &vertex_input_state;
	pipeline_create_info.pInputAssemblyState = &input_assembly_state;
	pipeline_create_info.pRasterizationState = &rasterization_state;
	pipeline_create_info.pColorBlendState = &color_blend_state;
	pipeline_create_info.pMultisampleState = &multisample_state;
	pipeline_create_info.pViewportState = &viewport_state;
	pipeline_create_info.pDepthStencilState = &depth_stencil_state;
	pipeline_create_info.pDynamicState = &dynamic_state;
	pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
	pipeline_create_info.pStages = shader_stages.data();

	VK_CHECK(vkCreateGraphicsPipelines(get_device().get_handle(), pipeline_cache, 1, &pipeline_create_info, nullptr, &pipeline));
}

void hack_push_constant::hack_prepare()
{
	prepare_aligned_models();
	setup_descriptor_set_layout();
	prepare_pipelines();
	setup_descriptor_pool();
	setup_descriptor_set();
	build_command_buffers();
}

void hack_push_constant::hack_render(VkCommandBuffer &commandBuffer)
{
	draw(commandBuffer);
}

///
std::unique_ptr<vkb::VulkanSampleC> create_hack_push_constant()
{
	return std::make_unique<hack_push_constant>();
}
