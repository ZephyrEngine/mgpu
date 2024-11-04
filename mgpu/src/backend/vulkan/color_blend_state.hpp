
#pragma once

#include <mgpu/mgpu.h>

#include <atom/vector_n.hpp>
#include <vulkan/vulkan.h>

#include "backend/color_blend_state.hpp"
#include "common/limits.hpp"

namespace mgpu::vulkan {

class ColorBlendState final : public ColorBlendStateBase {
  public:
    explicit ColorBlendState(const MGPUColorBlendStateCreateInfo& create_info);

    [[nodiscard]] const VkPipelineColorBlendStateCreateInfo& GetVkColorBlendState() const {
      return m_vk_color_blend_state;
    }

  private:
    VkPipelineColorBlendStateCreateInfo m_vk_color_blend_state{};
    atom::Vector_N<VkPipelineColorBlendAttachmentState, limits::max_color_attachments> m_vk_color_blend_attachment_states{};
};

} // namespace mgpu::vulkan
