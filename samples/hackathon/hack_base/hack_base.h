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

#pragma once

#include "TimeMeasurements.h"
#include "api_vulkan_sample.h"

/// 0 =   20 triangles,   12 vertices
/// 1 =   80 triangles,   42 vertices
/// 2 =  320 triangles,  162 vertices
/// 3 = 1280 triangles,  642 vertices
/// 4 = 5120 triangles, 2562 vertices
#define SPHERE_SUBDIVIDES 3
#define OBJECT_INSTANCES 1000

class hack_base : public ApiVulkanSample
{
  protected:
	void *aligned_alloc(size_t size, size_t alignment);
	void  aligned_free(void *data);

	struct Vertex
	{
		float pos[3];
		float color[3];
	};

	struct UboVS
	{
		glm::mat4 projection;
		glm::mat4 view;
	} ubo_vs;

	struct ViewUniformBuffer
	{
		std::unique_ptr<vkb::core::BufferC> view;
	} view_uniform_buffer;

  public:
	hack_base();
	virtual ~hack_base();

	void generate_cube(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
	Vertex generate_new_sphere_vertex(const Vertex &vertex1, const Vertex &vertex2, float radius) const;
	void generate_sphere(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
	void generate_model();
	void generate_rotations();
	void update_rotation(float delta_time);

	void begin_command_buffer(VkCommandBuffer &commandBuffer, VkFramebuffer &frameBuffer);
	void end_command_buffer(VkCommandBuffer &commandBuffer);

	void prepare_view_uniform_buffer();
	void update_view_uniform_buffer();

	void prepare_gpu_query_pool();
	void retrieve_gpu_results();

	void         build_command_buffers() override{/* no op - We are dynamically building the command buffers every frame. */};
	virtual bool prepare(const vkb::ApplicationOptions &options) override;
	virtual void render(float delta_time) override;
	virtual bool resize(const uint32_t width, const uint32_t height) override;

	// Replacement for `prepare` base interface
	virtual void hack_prepare() {};
	// Inject commands into command buffer before rendering
	virtual void hack_update(VkCommandBuffer& commandBuffer);
	// Replacement for `render` base interface
	virtual void hack_render(VkCommandBuffer &commandBuffer){};

  protected:
	// The cube
	std::unique_ptr<vkb::core::BufferC> vertex_buffer;
	std::unique_ptr<vkb::core::BufferC> index_buffer;
	uint32_t                            index_count = 0;

	// Store random per-object rotations for the models
	glm::vec3 rotations[OBJECT_INSTANCES];
	glm::vec3 rotation_speeds[OBJECT_INSTANCES];
	float     animation_timer = 0.0f;

	// Alignment setup calls for the test cases
	size_t alignment;
	void  *aligned_models = nullptr;

	void       prepare_aligned_models(size_t alignment = sizeof(glm::mat4), size_t *out_buffer_size = nullptr);
	glm::mat4 *get_aligned_model(size_t index);

	// Pipeline defaults
	VkPipelineInputAssemblyStateCreateInfo         input_assembly_state;
	VkPipelineRasterizationStateCreateInfo         rasterization_state;
	VkPipelineColorBlendAttachmentState            blend_attachment_state;
	VkPipelineColorBlendStateCreateInfo            color_blend_state;
	VkPipelineDepthStencilStateCreateInfo          depth_stencil_state;
	VkPipelineViewportStateCreateInfo              viewport_state;
	VkPipelineMultisampleStateCreateInfo           multisample_state;
	std::vector<VkDynamicState>                    dynamic_state_enables;
	VkPipelineDynamicStateCreateInfo               dynamic_state;
	std::vector<VkVertexInputBindingDescription>   vertex_input_bindings;
	std::vector<VkVertexInputAttributeDescription> vertex_input_attributes;
	VkPipelineVertexInputStateCreateInfo           vertex_input_state;

	// GPU timing
	VkQueryPool gpu_query_pool;
	uint32_t    gpu_pool_size;
	float       gpu_nano_per_ticks;
	uint64_t    gpu_timestamp_valid_bits;

	// Timing utilities
	TimeMeasurements mTimeMeasurements = TimeMeasurements();
	uint16_t         mFrameNumber      = 0;
};

std::unique_ptr<vkb::VulkanSampleC> create_hack_base();
