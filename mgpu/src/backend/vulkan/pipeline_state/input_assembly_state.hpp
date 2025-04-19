
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>

#include "backend/pipeline_state/input_assembly_state.hpp"

namespace mgpu::vulkan {

class InputAssemblyState final : public InputAssemblyStateBase {
  public:
    explicit InputAssemblyState(const MGPUInputAssemblyStateCreateInfo& create_info);

    [[nodiscard]] const VkPipelineInputAssemblyStateCreateInfo& GetVkInputAssemblyState() const {
      return m_vk_input_assembly_state;
    }

  private:
    VkPipelineInputAssemblyStateCreateInfo m_vk_input_assembly_state{};
};

} // namespace mgpu::vulkan
