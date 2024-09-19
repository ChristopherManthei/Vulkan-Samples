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

#include "../hack_multiple_uniform_buffer/hack_multiple_uniform_buffer.h"

class hack_multiple_uniform_buffer_updatebuffer : public hack_multiple_uniform_buffer
{
protected:
public:
  hack_multiple_uniform_buffer_updatebuffer();

  void prepare_uniform_buffer_gpu();
  virtual void hack_prepare() override;
  virtual void hack_update(VkCommandBuffer& commandBuffer) override;
  virtual void hack_render(VkCommandBuffer& commandBuffer) override;
};

std::unique_ptr<vkb::VulkanSampleC> create_hack_multiple_uniform_buffer_updatebuffer();
