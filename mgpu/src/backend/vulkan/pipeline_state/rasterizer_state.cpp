
#include "backend/vulkan/conversion.hpp"
#include "rasterizer_state.hpp"

namespace mgpu::vulkan {

RasterizerState::RasterizerState(const MGPURasterizerStateCreateInfo& create_info) {
  m_vk_rasterization_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthClampEnable = create_info.depth_clamp_enable,
    .rasterizerDiscardEnable = create_info.rasterizer_discard_enable,
    .polygonMode = MGPUPolygonModeToVkPolygonMode(create_info.polygon_mode),
    .cullMode = MGPUCullModeToVkCullMode(create_info.cull_mode),
    .frontFace = MGPUFrontFaceToVkFrontFace(create_info.front_face),
    .depthBiasEnable = create_info.depth_bias_enable,
    .depthBiasConstantFactor = create_info.depth_bias_constant_factor,
    .depthBiasClamp = create_info.depth_bias_clamp,
    .depthBiasSlopeFactor = create_info.depth_bias_slope_factor,
    .lineWidth = create_info.line_width
  };
}

} // namespace mgpu::vulkan
