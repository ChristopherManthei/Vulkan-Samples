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

#include "hack_multiple_uniform_buffer_descriptor_scratch.h"

#include "benchmark_mode/benchmark_mode.h"

hack_multiple_uniform_buffer_descriptor_scratch::hack_multiple_uniform_buffer_descriptor_scratch()
{
  title = "Hack: Multiple uniform buffers descriptor scratch";
}

void hack_multiple_uniform_buffer_descriptor_scratch::hack_update(VkCommandBuffer& commandBuffer)
{
  vkResetDescriptorPool(get_device().get_handle(), descriptor_pool, 0);
  memset(&descriptor_set[0], 0, OBJECT_INSTANCES * sizeof(VkDescriptorSet));
}

void hack_multiple_uniform_buffer_descriptor_scratch::hack_render(VkCommandBuffer& commandBuffer)
{
  if (!paused)
  {
    update_uniform_buffer();
  }

  VkDescriptorSetAllocateInfo alloc_info =
    vkb::initializers::descriptor_set_allocate_info(
      descriptor_pool,
      &descriptor_set_layout,
      1);

  VkDescriptorBufferInfo view_buffer_descriptor = create_descriptor(*view_uniform_buffer.view);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkDeviceSize offsets[1] = { 0 };
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertex_buffer->get(), offsets);
  vkCmdBindIndexBuffer(commandBuffer, index_buffer->get_handle(), 0, VK_INDEX_TYPE_UINT32);

  for (size_t j = 0; j < OBJECT_INSTANCES; j++)
  {
    VkDescriptorBufferInfo cube_buffer_descriptor = create_descriptor(*uniform_buffers.single[j], alignment);

    VK_CHECK(vkAllocateDescriptorSets(get_device().get_handle(), &alloc_info, &descriptor_set[j]));
    std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
      // Binding 0 : Projection/View matrix uniform buffer
      vkb::initializers::write_descriptor_set(descriptor_set[j], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &view_buffer_descriptor),
      // Binding 1 : Instance matrix as dynamic uniform buffer
      vkb::initializers::write_descriptor_set(descriptor_set[j], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &cube_buffer_descriptor),
    };

    vkUpdateDescriptorSets(get_device().get_handle(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, NULL);

    // Bind the descriptor set for rendering a mesh using the dynamic offset
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set[j], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, index_count, 1, 0, 0, 0);
  }

  draw(commandBuffer);
}

/// 
std::unique_ptr<vkb::VulkanSampleC> create_hack_multiple_uniform_buffer_descriptor_scratch()
{
  return std::make_unique<hack_multiple_uniform_buffer_descriptor_scratch>();
}
