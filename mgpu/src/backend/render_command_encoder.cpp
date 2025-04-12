
#include "command_list.hpp"
#include "render_command_encoder.hpp"

namespace mgpu {

void RenderCommandEncoder::CmdUseShaderProgram(ShaderProgramBase* shader_program) {
  m_command_list->CmdUseShaderProgram(shader_program);
}

void RenderCommandEncoder::CmdUseRasterizerState(RasterizerStateBase* rasterizer_state) {
  m_command_list->CmdUseRasterizerState(rasterizer_state);
}

void RenderCommandEncoder::CmdUseInputAssemblyState(InputAssemblyStateBase* input_assembly_state) {
  m_command_list->CmdUseInputAssemblyState(input_assembly_state);
}

void RenderCommandEncoder::CmdUseColorBlendState(ColorBlendStateBase* color_blend_state) {
  m_command_list->CmdUseColorBlendState(color_blend_state);
}

void RenderCommandEncoder::CmdUseVertexInputState(VertexInputStateBase* vertex_input_state) {
  m_command_list->CmdUseVertexInputState(vertex_input_state);
}

void RenderCommandEncoder::CmdUseDepthStencilState(DepthStencilStateBase* depth_stencil_state) {
  m_command_list->CmdUseDepthStencilState(depth_stencil_state);
}

void RenderCommandEncoder::CmdSetViewport(f32 x, f32 y, f32 width, f32 height) {
  m_command_list->CmdSetViewport(x, y, width, height);
}

void RenderCommandEncoder::CmdSetScissor(i32 x, i32 y, u32 width, u32 height) {
  m_command_list->CmdSetScissor(x, y, width, height);
}

void RenderCommandEncoder::CmdBindVertexBuffer(u32 binding, BufferBase* buffer, u64 buffer_offset) {
  m_command_list->CmdBindVertexBuffer(binding, buffer, buffer_offset);
}

void RenderCommandEncoder::CmdBindResourceSet(u32 index, ResourceSetBase* resource_set) {
  m_command_list->CmdBindResourceSet(index, resource_set);
}

void RenderCommandEncoder::CmdDraw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) {
  m_command_list->CmdDraw(vertex_count, instance_count, first_vertex, first_instance);
}

void RenderCommandEncoder::Close() {
  m_command_list->CmdEndRenderPass();
}

} // namespace mgpu
