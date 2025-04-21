
#include <mgpu/mgpu.h>

#include "backend/command_list/render_command_encoder.hpp"

extern "C" {

void mgpuRenderCommandEncoderCmdUseShaderProgram(MGPURenderCommandEncoder render_command_encoder, MGPUShaderProgram shader_program) {
  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdUseShaderProgram((mgpu::ShaderProgramBase*)shader_program);
}

void mgpuRenderCommandEncoderCmdUseRasterizerState(MGPURenderCommandEncoder render_command_encoder, MGPURasterizerState rasterizer_state) {
  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdUseRasterizerState((mgpu::RasterizerStateBase*)rasterizer_state);
}

void mgpuRenderCommandEncoderCmdUseInputAssemblyState(MGPURenderCommandEncoder render_command_encoder, MGPUInputAssemblyState input_assembly_state) {
  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdUseInputAssemblyState((mgpu::InputAssemblyStateBase*)input_assembly_state);
}

void mgpuRenderCommandEncoderCmdUseColorBlendState(MGPURenderCommandEncoder render_command_encoder, MGPUColorBlendState color_blend_state) {
  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdUseColorBlendState((mgpu::ColorBlendStateBase*)color_blend_state);
}

void mgpuRenderCommandEncoderCmdUseVertexInputState(MGPURenderCommandEncoder render_command_encoder, MGPUVertexInputState vertex_input_state) {
  // TODO(fleroviux): validate that vertex input state is compatible with the current vertex shader.

  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdUseVertexInputState((mgpu::VertexInputStateBase*)vertex_input_state);
}

void mgpuRenderCommandEncoderCmdUseDepthStencilState(MGPURenderCommandEncoder render_command_encoder, MGPUDepthStencilState depth_stencil_state) {
  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdUseDepthStencilState((mgpu::DepthStencilStateBase*)depth_stencil_state);
}

void mgpuRenderCommandEncoderCmdSetViewport(MGPURenderCommandEncoder render_command_encoder, float x, float y, float width, float height) {
  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdSetViewport(x, y, width, height);
}

void mgpuRenderCommandEncoderCmdSetScissor(MGPURenderCommandEncoder render_command_encoder, int32_t x, int32_t y, uint32_t width, uint32_t height) {
  // TODO(fleroviux): implement validation?
  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdSetScissor(x, y, width, height);
}

void mgpuRenderCommandEncoderCmdBindVertexBuffer(MGPURenderCommandEncoder render_command_encoder, uint32_t binding, MGPUBuffer buffer, uint64_t buffer_offset) {
  // TODO(fleroviux): implement validation? i.e. buffer usage

  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdBindVertexBuffer(binding, (mgpu::BufferBase*)buffer, buffer_offset);
}

void mgpuRenderCommandEncoderCmdBindResourceSet(MGPURenderCommandEncoder render_command_encoder, uint32_t index, MGPUResourceSet resource_set) {
  // TODO(fleroviux): implement validation?
  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdBindResourceSet(index, (mgpu::ResourceSetBase*)resource_set);
}

void mgpuRenderCommandEncoderCmdDraw(MGPURenderCommandEncoder render_command_encoder, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
  // TODO(fleroviux): validate that all vertex buffer bindings have something bound?

  ((mgpu::RenderCommandEncoder*)render_command_encoder)->CmdDraw(vertex_count, instance_count, first_vertex, first_instance);
}

void mgpuRenderCommandEncoderClose(MGPURenderCommandEncoder render_command_encoder) {
  ((mgpu::RenderCommandEncoder*)render_command_encoder)->Close();
}

} // extern "C"
