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

#include "hack_dynamic_uniform_buffer_gpu.h"

#include "benchmark_mode/benchmark_mode.h"

hack_dynamic_uniform_buffer_gpu::hack_dynamic_uniform_buffer_gpu()
{
  // Force GLSL to not have to implement all shaders twice.
  set_shading_language(vkb::ShadingLanguage::GLSL);

  title = "Hack: Dynamic uniform buffers GPU";
}

void hack_dynamic_uniform_buffer_gpu::prepare_dynamic_uniform_buffer_gpu()
{
  uniform_buffers_gpu.dynamic = std::make_unique<vkb::core::BufferC>(get_device(),
    OBJECT_INSTANCES * alignment,
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VMA_MEMORY_USAGE_GPU_ONLY);
}

void hack_dynamic_uniform_buffer_gpu::setup_descriptor_set_gpu()
{
  VkDescriptorSetAllocateInfo alloc_info =
    vkb::initializers::descriptor_set_allocate_info(
      descriptor_pool,
      &descriptor_set_layout,
      1);

  VK_CHECK(vkAllocateDescriptorSets(get_device().get_handle(), &alloc_info, &descriptor_set));

  VkDescriptorBufferInfo view_buffer_descriptor = create_descriptor(*view_uniform_buffer.view);

  // Pass the  actual dynamic alignment as the descriptor's size
  VkDescriptorBufferInfo dynamic_buffer_descriptor = create_descriptor(*uniform_buffers_gpu.dynamic, alignment);

  std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
    // Binding 0 : Projection/View matrix uniform buffer
    vkb::initializers::write_descriptor_set(descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &view_buffer_descriptor),
    // Binding 1 : Instance matrix as dynamic uniform buffer
    vkb::initializers::write_descriptor_set(descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, &dynamic_buffer_descriptor),
  };

  vkUpdateDescriptorSets(get_device().get_handle(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, NULL);
}

void hack_dynamic_uniform_buffer_gpu::hack_prepare()
{
  prepare_dynamic_uniform_buffer();
  prepare_dynamic_uniform_buffer_gpu();
  setup_descriptor_set_layout();
  prepare_pipelines();
  setup_descriptor_pool();
  // Skip the descriptor creation based on the CPU buffer and create descriptors to the GPU memory instead.
  //setup_descriptor_set();
  setup_descriptor_set_gpu();
  build_command_buffers();
}

void hack_dynamic_uniform_buffer_gpu::hack_update(VkCommandBuffer& commandBuffer)
{
  {
    // Barrier CPU Write -> Transfer Read
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_HOST_WRITE_BIT;
    memoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_HOST_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
  }
  // Copy
  VkBufferCopy cpy;
  cpy.srcOffset = 0;
  cpy.dstOffset = 0;
  cpy.size = uniform_buffers.dynamic->get_size();

  vkCmdCopyBuffer(commandBuffer, uniform_buffers.dynamic->get_handle(), uniform_buffers_gpu.dynamic->get_handle(), 1, &cpy);
  {
    // Barrier Transfer Write -> Shader Read
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
  }
}

/// 
std::unique_ptr<vkb::VulkanSampleC> create_hack_dynamic_uniform_buffer_gpu()
{
  return std::make_unique<hack_dynamic_uniform_buffer_gpu>();
}
