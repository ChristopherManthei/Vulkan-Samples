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

#include "hack_dynamic_uniform_buffer_up_multi.h"

#include "benchmark_mode/benchmark_mode.h"

hack_dynamic_uniform_buffer_up_multi::hack_dynamic_uniform_buffer_up_multi()
{
  // Force GLSL to not have to implement all shaders twice.
  set_shading_language(vkb::ShadingLanguage::GLSL);

  title = "Hack: Dynamic uniform buffers upload multiple";
}

void hack_dynamic_uniform_buffer_up_multi::update_dynamic_uniform_buffer()
{
  auto dim = static_cast<uint32_t>(pow(OBJECT_INSTANCES, (1.0f / 3.0f)));
  auto fdim = static_cast<float>(dim);

  const uint8_t* sourceData = reinterpret_cast<const uint8_t*>(aligned_models);
  uint8_t* mapped_data = uniform_buffers.dynamic->map();

  for (uint32_t x = 0; x < dim; x++)
  {
    for (uint32_t y = 0; y < dim; y++)
    {
      for (uint32_t z = 0; z < dim; z++)
      {
        auto index = x * dim * dim + y * dim + z;
        auto offset = index * alignment;
        std::copy(sourceData + offset, sourceData + offset + alignment, mapped_data + offset);
      }
    }
  }

  uniform_buffers.dynamic->unmap();
  // Flush to make changes visible to the device
  uniform_buffers.dynamic->flush();
}

/// 
std::unique_ptr<vkb::VulkanSampleC> create_hack_dynamic_uniform_buffer_up_multi()
{
  return std::make_unique<hack_dynamic_uniform_buffer_up_multi>();
}
