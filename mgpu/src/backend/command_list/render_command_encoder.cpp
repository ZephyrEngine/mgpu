
#include "command_list.hpp"
#include "render_command_encoder.hpp"

namespace mgpu {

void RenderCommandEncoder::CmdUseShaderProgram(ShaderProgramBase* shader_program) {
  m_command_list->Push<UseShaderProgramCommand>(shader_program);
}

void RenderCommandEncoder::CmdUseRasterizerState(RasterizerStateBase* rasterizer_state) {
  m_command_list->Push<UseRasterizerStateCommand>(rasterizer_state);
}

void RenderCommandEncoder::CmdUseInputAssemblyState(InputAssemblyStateBase* input_assembly_state) {
  m_command_list->Push<UseInputAssemblyStateCommand>(input_assembly_state);
}

void RenderCommandEncoder::CmdUseColorBlendState(ColorBlendStateBase* color_blend_state) {
  m_command_list->Push<UseColorBlendStateCommand>(color_blend_state);
}

void RenderCommandEncoder::CmdUseVertexInputState(VertexInputStateBase* vertex_input_state) {
  m_command_list->Push<UseVertexInputStateCommand>(vertex_input_state);
}

void RenderCommandEncoder::CmdUseDepthStencilState(DepthStencilStateBase* depth_stencil_state) {
  m_command_list->Push<UseDepthStencilStateCommand>(depth_stencil_state);
}

void RenderCommandEncoder::CmdSetViewport(f32 x, f32 y, f32 width, f32 height) {
  m_command_list->Push<SetViewportCommand>(x, y, width, height);
}

void RenderCommandEncoder::CmdSetScissor(i32 x, i32 y, u32 width, u32 height) {
  m_command_list->Push<SetScissorCommand>(x, y, width, height);
}

void RenderCommandEncoder::CmdBindVertexBuffer(u32 binding, BufferBase* buffer, u64 buffer_offset) {
  m_command_list->Push<BindVertexBufferCommand>(binding, buffer, buffer_offset);
}

void RenderCommandEncoder::CmdBindResourceSet(u32 index, ResourceSetBase* resource_set) {
  m_command_list->Push<BindResourceSetCommand>(index, resource_set);
}

void RenderCommandEncoder::CmdDraw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) {
  m_command_list->Push<DrawCommand>(vertex_count, instance_count, first_vertex, first_instance);
}

void RenderCommandEncoder::Close() {
  m_command_list->Push<EndRenderPassCommand>();
  m_command_list->m_state.inside_render_pass = false;
  m_command_list = nullptr;
}

} // namespace mgpu
