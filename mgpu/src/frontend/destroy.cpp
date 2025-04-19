
#include <mgpu/mgpu.h>

#include "backend/pipeline_state/color_blend_state.hpp"
#include "backend/pipeline_state/depth_stencil_state.hpp"
#include "backend/pipeline_state/input_assembly_state.hpp"
#include "backend/pipeline_state/rasterizer_state.hpp"
#include "backend/pipeline_state/shader_module.hpp"
#include "backend/pipeline_state/shader_program.hpp"
#include "backend/pipeline_state/vertex_input_state.hpp"
#include "backend/buffer.hpp"
#include "backend/command_list.hpp"
#include "backend/device.hpp"
#include "backend/instance.hpp"
#include "backend/resource_set_layout.hpp"
#include "backend/resource_set.hpp"
#include "backend/sampler.hpp"
#include "backend/surface.hpp"
#include "backend/swap_chain.hpp"
#include "backend/texture.hpp"
#include "backend/texture_view.hpp"

extern "C" {

void mgpuInstanceDestroy(MGPUInstance instance) {
  delete (mgpu::InstanceBase*)instance;
}

void mgpuDeviceDestroy(MGPUDevice device) {
  delete (mgpu::DeviceBase*)device;
}

void mgpuBufferDestroy(MGPUBuffer buffer) {
  delete (mgpu::BufferBase*)buffer;
}

void mgpuTextureDestroy(MGPUTexture texture) {
  delete (mgpu::TextureBase*)texture;
}

void mgpuTextureViewDestroy(MGPUTextureView texture_view) {
  delete (mgpu::TextureViewBase*)texture_view;
}

void mgpuSamplerDestroy(MGPUSampler sampler) {
  delete (mgpu::SamplerBase*)sampler;
}

void mgpuResourceSetLayoutDestroy(MGPUResourceSetLayout resource_set_layout) {
  delete (mgpu::ResourceSetLayoutBase*)resource_set_layout;
}

void mgpuResourceSetDestroy(MGPUResourceSet resource_set) {
  delete (mgpu::ResourceSetBase*)resource_set;
}

void mgpuShaderModuleDestroy(MGPUShaderModule shader_module) {
  delete (mgpu::ShaderModuleBase*)shader_module;
}

void mgpuShaderProgramDestroy(MGPUShaderProgram shader_program) {
  delete (mgpu::ShaderProgramBase*)shader_program;
}

void mgpuRasterizerStateDestroy(MGPURasterizerState rasterizer_state) {
  delete (mgpu::RasterizerStateBase*)rasterizer_state;
}

void mgpuInputAssemblyStateDestroy(MGPUInputAssemblyState input_assembly_state) {
  delete (mgpu::InputAssemblyStateBase*)input_assembly_state;
}

void mgpuColorBlendStateDestroy(MGPUColorBlendState color_blend_state) {
  delete (mgpu::ColorBlendStateBase*)color_blend_state;
}

void mgpuVertexInputStateDestroy(MGPUVertexInputState vertex_input_state) {
  delete (mgpu::VertexInputStateBase*)vertex_input_state;
}

void mgpuDepthStencilStateDestroy(MGPUDepthStencilState depth_stencil_state) {
  delete (mgpu::DepthStencilStateBase*)depth_stencil_state;
}

void mgpuCommandListDestroy(MGPUCommandList command_list) {
  delete (mgpu::CommandList*)command_list;
}

void mgpuSurfaceDestroy(MGPUSurface surface) {
  delete (mgpu::SurfaceBase*)surface;
}

void mgpuSwapChainDestroy(MGPUSwapChain swap_chain) {
  delete (mgpu::SwapChainBase*)swap_chain;
}

} // extern "C"
