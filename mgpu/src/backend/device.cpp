
#include <atom/vector_n.hpp>

#include "device.hpp"

namespace mgpu {

RasterizerStateBase* DeviceBase::GetDefaultRasterizerState() {
  if(m_default_rasterizer_state == nullptr) {
    m_default_rasterizer_state = CreateRasterizerState({
      .depth_clamp_enable = false,
      .rasterizer_discard_enable = false,
      .polygon_mode = MGPU_POLYGON_MODE_FILL,
      .cull_mode = 0, // disables face culling
      .front_face = MGPU_FRONT_FACE_COUNTER_CLOCKWISE,
      .depth_bias_enable = false,
      .depth_bias_constant_factor = 0.0f,
      .depth_bias_clamp = 0.0f,
      .depth_bias_slope_factor = 0.0f,
      .line_width = 1.0f
    }).Unwrap();
  }
  return m_default_rasterizer_state;
}

InputAssemblyStateBase* DeviceBase::GetDefaultInputAssemblyState() {
  if(m_default_input_assembly_state == nullptr) {
    m_default_input_assembly_state = CreateInputAssemblyState({
      .topology = MGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitive_restart_enable = false
    }).Unwrap();
  }
  return m_default_input_assembly_state;
}

VertexInputStateBase* DeviceBase::GetDefaultVertexInputState() {
  if(m_default_vertex_input_state == nullptr) {
    m_default_vertex_input_state = CreateVertexInputState({
      .binding_count = 0u,
      .bindings = nullptr,
      .attribute_count = 0u,
      .attributes = nullptr
    }).Unwrap();
  }
  return m_default_vertex_input_state;
}

DepthStencilStateBase* DeviceBase::GetDefaultDepthStencilState() {
  if(m_default_depth_stencil_state == nullptr) {
    m_default_depth_stencil_state = CreateDepthStencilState({
      .depth_test_enable = false,
      .depth_write_enable = false,
      .depth_compare_op = MGPU_COMPARE_OP_ALWAYS,
      .stencil_test_enable = false,
      .stencil_front = {},
      .stencil_back = {}
    }).Unwrap();
  }
  return m_default_depth_stencil_state;
}

ColorBlendStateBase* DeviceBase::GetDefaultColorBlendState(u32 attachment_count) {
  auto& default_color_blend_state = m_default_color_blend_states[attachment_count];
  if(default_color_blend_state == nullptr) {
    atom::Vector_N<MGPUColorBlendAttachmentState, limits::max_color_attachments> attachment_blend_states{};
    for(size_t i = 0; i < attachment_count; i++) {
      attachment_blend_states.PushBack({
        .blend_enable = false,
        .src_color_blend_factor = MGPU_BLEND_FACTOR_SRC_ALPHA,
        .dst_color_blend_factor = MGPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .color_blend_op = MGPU_BLEND_OP_ADD,
        .src_alpha_blend_factor = MGPU_BLEND_FACTOR_ONE,
        .dst_alpha_blend_factor = MGPU_BLEND_FACTOR_ONE,
        .alpha_blend_op = MGPU_BLEND_OP_MAX,
        .color_write_mask = 0b1111
      });
    }

    default_color_blend_state = CreateColorBlendState({
      .attachment_count = attachment_count,
      .attachments = attachment_blend_states.Data()
    }).Unwrap();
  }
  return default_color_blend_state;
}

} // namespace mgpu
