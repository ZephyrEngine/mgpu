
#pragma once

#include <mgpu/mgpu.h>
#include <vulkan/vulkan.h>

#include "backend/depth_stencil_state.hpp"

namespace mgpu::vulkan {

class DepthStencilState : public DepthStencilStateBase {
  public:
    explicit DepthStencilState(const MGPUDepthStencilStateCreateInfo& create_info);

    [[nodiscard]] const VkPipelineDepthStencilStateCreateInfo& GetVkDepthStencilState() const {
      return m_vk_depth_stencil_state_create_info;
    }

  private:
    VkPipelineDepthStencilStateCreateInfo m_vk_depth_stencil_state_create_info;
};

} // namespace mgpu::vulkan
