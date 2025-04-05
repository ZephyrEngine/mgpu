
#include "conversion.hpp"
#include "depth_stencil_state.hpp"

namespace mgpu::vulkan {

DepthStencilState::DepthStencilState(const MGPUDepthStencilStateCreateInfo& create_info) {
  const auto MGPUStencilFaceStateToVkStencilOpState = [](const MGPUStencilFaceState& stencil_face_state) {
    return VkStencilOpState{
      .failOp = MGPUStencilOpToVkStencilOp(stencil_face_state.fail_op),
      .passOp = MGPUStencilOpToVkStencilOp(stencil_face_state.pass_op),
      .depthFailOp = MGPUStencilOpToVkStencilOp(stencil_face_state.depth_fail_op),
      .compareOp = MGPUCompareOpToVkCompareOp(stencil_face_state.compare_op),
      .compareMask = stencil_face_state.read_mask,
      .writeMask = stencil_face_state.write_mask,
      .reference = stencil_face_state.reference
    };
  };

  // TODO(fleroviux): should the depth bounds test actually be enabled?
  m_vk_depth_stencil_state_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthTestEnable = create_info.depth_test_enable,
    .depthWriteEnable = create_info.depth_write_enable,
    .depthCompareOp = MGPUCompareOpToVkCompareOp(create_info.depth_compare_op),
    .depthBoundsTestEnable = VK_TRUE,

    .stencilTestEnable = create_info.stencil_test_enable,
    .front = MGPUStencilFaceStateToVkStencilOpState(create_info.stencil_front),
    .back = MGPUStencilFaceStateToVkStencilOpState(create_info.stencil_back),
    .minDepthBounds = 0.f,
    .maxDepthBounds = 1.f
  };
}

} // namespace mgpu::vulkan