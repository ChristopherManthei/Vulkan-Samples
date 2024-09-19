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

#include "hack_multiple_uniform_buffer_updatebuffer.h"

#include "benchmark_mode/benchmark_mode.h"

hack_multiple_uniform_buffer_updatebuffer::hack_multiple_uniform_buffer_updatebuffer()
{
  title = "Hack: Multiple uniform buffers updatebuffer";
}

void hack_multiple_uniform_buffer_updatebuffer::prepare_uniform_buffer_gpu()
{
	prepare_aligned_models(sizeof(glm::mat4), nullptr);

	// Vertex shader uniform buffer block
	for (size_t i = 0; i < OBJECT_INSTANCES; i++)
	{
		uniform_buffers.single[i] = std::make_unique<vkb::core::BufferC>(get_device(),
			alignment,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY);
	}
}

void hack_multiple_uniform_buffer_updatebuffer::hack_prepare()
{
	//prepare_uniform_buffer();
	prepare_uniform_buffer_gpu();
	setup_descriptor_set_layout();
	prepare_pipelines();
	setup_descriptor_pool();
	setup_descriptor_set();
	build_command_buffers();
}

void hack_multiple_uniform_buffer_updatebuffer::hack_update(VkCommandBuffer& commandBuffer)
{
	for (size_t i = 0; i < OBJECT_INSTANCES; i++)
	{
		vkCmdUpdateBuffer(commandBuffer, uniform_buffers.single[i]->get_handle(), 0, alignment, get_aligned_model(i));
	}

  // Barrier
	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
	memoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
}

void hack_multiple_uniform_buffer_updatebuffer::hack_render(VkCommandBuffer& commandBuffer)
{
	draw(commandBuffer);
}

/// 
std::unique_ptr<vkb::VulkanSampleC> create_hack_multiple_uniform_buffer_updatebuffer()
{
  return std::make_unique<hack_multiple_uniform_buffer_updatebuffer>();
}
