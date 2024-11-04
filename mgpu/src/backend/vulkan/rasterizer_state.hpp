
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>

#include "backend/rasterizer_state.hpp"

namespace mgpu::vulkan {

class RasterizerState final : public RasterizerStateBase {
  public:
    explicit RasterizerState(const MGPURasterizerStateCreateInfo& create_info);

    [[nodiscard]] const VkPipelineRasterizationStateCreateInfo& GetVkRasterizationState() const {
      return m_vk_rasterization_state;
    }

  private:
    VkPipelineRasterizationStateCreateInfo m_vk_rasterization_state{};
};

} // namespace mgpu::vulkan
