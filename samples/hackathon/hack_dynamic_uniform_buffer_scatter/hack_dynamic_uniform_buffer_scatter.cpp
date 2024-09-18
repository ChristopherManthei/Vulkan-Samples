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

#include "hack_dynamic_uniform_buffer_scatter.h"

#include "benchmark_mode/benchmark_mode.h"

hack_dynamic_uniform_buffer_scatter::hack_dynamic_uniform_buffer_scatter()
{
  // Force GLSL to not have to implement all shaders twice.
  set_shading_language(vkb::ShadingLanguage::GLSL);

  title = "Hack: Dynamic uniform buffers scatter";
}

void hack_dynamic_uniform_buffer_scatter::hack_render(VkCommandBuffer& commandBuffer)
{
	if (!paused)
	{
		update_dynamic_uniform_buffer();
	}

	draw_scattered(commandBuffer);
}

/// 
std::unique_ptr<vkb::VulkanSampleC> create_hack_dynamic_uniform_buffer_scatter()
{
  return std::make_unique<hack_dynamic_uniform_buffer_scatter>();
}
