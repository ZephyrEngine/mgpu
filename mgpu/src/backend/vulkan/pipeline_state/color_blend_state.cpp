
#include "backend/vulkan/conversion.hpp"
#include "color_blend_state.hpp"

namespace mgpu::vulkan {

ColorBlendState::ColorBlendState(const MGPUColorBlendStateCreateInfo& create_info) {
  for(size_t i = 0; i < create_info.attachment_count; i++) {
    const MGPUColorBlendAttachmentState& attachment_state = create_info.attachments[i];
    m_vk_color_blend_attachment_states.PushBack({
      .blendEnable = attachment_state.blend_enable,
      .srcColorBlendFactor = MGPUBlendFactorToVkBlendFactor(attachment_state.src_color_blend_factor),
      .dstColorBlendFactor = MGPUBlendFactorToVkBlendFactor(attachment_state.dst_color_blend_factor),
      .colorBlendOp = MGPUBlendOpToVkBlendOp(attachment_state.color_blend_op),
      .srcAlphaBlendFactor = MGPUBlendFactorToVkBlendFactor(attachment_state.src_alpha_blend_factor),
      .dstAlphaBlendFactor = MGPUBlendFactorToVkBlendFactor(attachment_state.dst_alpha_blend_factor),
      .alphaBlendOp = MGPUBlendOpToVkBlendOp(attachment_state.alpha_blend_op),
      .colorWriteMask = attachment_state.color_write_mask & 15
    });
  }

  m_vk_color_blend_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_CLEAR,
    .attachmentCount = (u32)m_vk_color_blend_attachment_states.Size(),
    .pAttachments = m_vk_color_blend_attachment_states.Data(),
    .blendConstants = {0.f, 0.f, 0.f, 0.f}
  };
}

} // namespace mgpu::vulkan
