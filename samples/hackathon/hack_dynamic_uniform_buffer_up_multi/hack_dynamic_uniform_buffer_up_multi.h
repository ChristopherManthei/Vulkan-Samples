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

#include "../hack_dynamic_uniform_buffer/hack_dynamic_uniform_buffer.h"

class hack_dynamic_uniform_buffer_up_multi : public hack_dynamic_uniform_buffer
{
public:
  hack_dynamic_uniform_buffer_up_multi();

  virtual void update_dynamic_uniform_buffer() override;
};

std::unique_ptr<vkb::VulkanSampleC> create_hack_dynamic_uniform_buffer_up_multi();
