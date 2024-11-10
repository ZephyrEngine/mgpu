
#include <mgpu/mgpu.h>

#include "backend/command_list.hpp"

extern "C" {

MGPUResult mgpuCommandListClear(MGPUCommandList command_list) {
  ((mgpu::CommandList*)command_list)->Clear();
  return MGPU_SUCCESS;
}

void mgpuCommandListCmdBeginRenderPass(MGPUCommandList command_list, const MGPURenderPassBeginInfo* begin_info) {
  /**
   * TODO(fleroviux): validate begin_info.
   *
   * Some ideas for validation:
   *  - Must have at least one attachment
   *  - Must not have more than the allowed number of color attachments
   *  - All attachments must have the same dimensions
   *  - All attachments must have RENDER_ATTACHMENT usage
   *  - All attachments must be of type MGPU_TEXTURE_VIEW_TYPE_2D (or MGPU_TEXTURE_VIEW_TYPE_2D_ARRAY, if we support layers in the future)
   *  - All color attachments must have a format with color aspect
   *  - The depth/stencil attachment must have a format with a depth or stencil aspect
   */

  ((mgpu::CommandList*)command_list)->CmdBeginRenderPass(*begin_info);
}

void mgpuCommandListCmdEndRenderPass(MGPUCommandList command_list) {
  ((mgpu::CommandList*)command_list)->CmdEndRenderPass();
}

void mgpuCommandListCmdUseShaderProgram(MGPUCommandList command_list, MGPUShaderProgram shader_program) {
  ((mgpu::CommandList*)command_list)->CmdUseShaderProgram((mgpu::ShaderProgramBase*)shader_program);
}

void mgpuCommandListCmdUseRasterizerState(MGPUCommandList command_list, MGPURasterizerState rasterizer_state) {
  ((mgpu::CommandList*)command_list)->CmdUseRasterizerState((mgpu::RasterizerStateBase*)rasterizer_state);
}

void mgpuCommandListCmdUseInputAssemblyState(MGPUCommandList command_list, MGPUInputAssemblyState input_assembly_state) {
  ((mgpu::CommandList*)command_list)->CmdUseInputAssemblyState((mgpu::InputAssemblyStateBase*)input_assembly_state);
}

void mgpuCommandListCmdUseColorBlendState(MGPUCommandList command_list, MGPUColorBlendState color_blend_state) {
  ((mgpu::CommandList*)command_list)->CmdUseColorBlendState((mgpu::ColorBlendStateBase*)color_blend_state);
}

void mgpuCommandListCmdUseVertexInputState(MGPUCommandList command_list, MGPUVertexInputState vertex_input_state) {
  ((mgpu::CommandList*)command_list)->CmdUseVertexInputState((mgpu::VertexInputStateBase*)vertex_input_state);
}

void mgpuCommandListCmdSetViewport(MGPUCommandList command_list, float x, float y, float width, float height) {
  ((mgpu::CommandList*)command_list)->CmdSetViewport(x, y, width, height);
}

void mgpuCommandListCmdSetScissor(MGPUCommandList command_list, int32_t x, int32_t y, uint32_t width, uint32_t height) {
  // TODO(fleroviux): implement validation?
  ((mgpu::CommandList*)command_list)->CmdSetScissor(x, y, width, height);
}

void mgpuCommandListBindVertexBuffer(MGPUCommandList command_list, uint32_t binding, MGPUBuffer buffer, uint64_t buffer_offset) {
  ((mgpu::CommandList*)command_list)->CmdBindVertexBuffer(binding, (mgpu::BufferBase*)buffer, buffer_offset);
}

void mgpuCommandListCmdDraw(MGPUCommandList command_list, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
  ((mgpu::CommandList*)command_list)->CmdDraw(vertex_count, instance_count, first_vertex, first_instance);
}

void mgpuCommandListDestroy(MGPUCommandList command_list) {
  delete (mgpu::CommandList*)command_list;
}

}  // extern "C"
