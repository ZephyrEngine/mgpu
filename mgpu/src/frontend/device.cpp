
#include <mgpu/mgpu.h>

#include "backend/command_list.hpp"
#include "backend/device.hpp"
#include "validation/buffer.hpp"
#include "validation/sampler.hpp"
#include "validation/shader_program.hpp"
#include "validation/texture.hpp"

extern "C" {

MGPUQueue mgpuDeviceGetQueue(MGPUDevice device, MGPUQueueType queue_type) {
  return (MGPUQueue)((mgpu::DeviceBase*)device)->GetQueue(queue_type);
}

MGPUResult mgpuDeviceCreateBuffer(MGPUDevice device, const MGPUBufferCreateInfo* create_info, MGPUBuffer* buffer) {
  MGPU_FORWARD_ERROR(validate_buffer_size(create_info->size));
  MGPU_FORWARD_ERROR(validate_buffer_usage(create_info->usage));

  mgpu::Result<mgpu::BufferBase*> cxx_buffer_result = ((mgpu::DeviceBase*)device)->CreateBuffer(*create_info);
  MGPU_FORWARD_ERROR(cxx_buffer_result.Code());
  *buffer = (MGPUBuffer)cxx_buffer_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateTexture(MGPUDevice device, const MGPUTextureCreateInfo* create_info, MGPUTexture* texture) {
  const auto cxx_device = (mgpu::DeviceBase*)device;

  MGPU_FORWARD_ERROR(validate_texture_format(create_info->format));
  MGPU_FORWARD_ERROR(validate_texture_type(create_info->type));
  MGPU_FORWARD_ERROR(validate_texture_usage(create_info->usage));
  MGPU_FORWARD_ERROR(validate_texture_extent(cxx_device->Limits(), create_info->type, create_info->extent, create_info->usage));
  MGPU_FORWARD_ERROR(validate_texture_mip_count(create_info->extent, create_info->mip_count));
  MGPU_FORWARD_ERROR(validate_texture_array_layer_count(cxx_device->Limits(), create_info->array_layer_count));

  mgpu::Result<mgpu::TextureBase*> cxx_texture_result = cxx_device->CreateTexture(*create_info);
  MGPU_FORWARD_ERROR(cxx_texture_result.Code());
  *texture = (MGPUTexture)cxx_texture_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateSampler(MGPUDevice device, const MGPUSamplerCreateInfo* create_info, MGPUSampler* sampler) {
  const auto cxx_device = (mgpu::DeviceBase*)device;

  MGPU_FORWARD_ERROR(validate_sampler_mip_lod_bias(cxx_device->Limits(), create_info->mip_lod_bias));
  MGPU_FORWARD_ERROR(validate_sampler_lod_clamp(create_info->min_lod, create_info->max_lod));
  if(create_info->anisotropy_enable) {
    MGPU_FORWARD_ERROR(validate_sampler_max_anisotropy(cxx_device->Limits(), create_info->max_anisotropy));
  }

  mgpu::Result<mgpu::SamplerBase*> cxx_sampler_result = cxx_device->CreateSampler(*create_info);
  MGPU_FORWARD_ERROR(cxx_sampler_result.Code());
  *sampler = (MGPUSampler)cxx_sampler_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateResourceSetLayout(MGPUDevice device, const MGPUResourceSetLayoutCreateInfo* create_info, MGPUResourceSetLayout* resource_set_layout) {
  mgpu::Result<mgpu::ResourceSetLayoutBase*> cxx_resource_set_layout_result = ((mgpu::DeviceBase*)device)->CreateResourceSetLayout(*create_info);
  MGPU_FORWARD_ERROR(cxx_resource_set_layout_result.Code());
  *resource_set_layout = (MGPUResourceSetLayout)cxx_resource_set_layout_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateResourceSet(MGPUDevice device, const MGPUResourceSetCreateInfo* create_info, MGPUResourceSet* resource_set) {
  mgpu::Result<mgpu::ResourceSetBase*> cxx_resource_set_result = ((mgpu::DeviceBase*)device)->CreateResourceSet(*create_info);
  MGPU_FORWARD_ERROR(cxx_resource_set_result.Code());
  *resource_set = (MGPUResourceSet)cxx_resource_set_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateShaderModule(MGPUDevice device, const uint32_t* spirv_code, size_t spirv_byte_size, MGPUShaderModule* shader_module) {
  mgpu::Result<mgpu::ShaderModuleBase*> cxx_shader_module_result = ((mgpu::DeviceBase*)device)->CreateShaderModule(spirv_code, spirv_byte_size);
  MGPU_FORWARD_ERROR(cxx_shader_module_result.Code());
  *shader_module = (MGPUShaderModule)cxx_shader_module_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateShaderProgram(MGPUDevice device, const MGPUShaderProgramCreateInfo* create_info, MGPUShaderProgram* shader_program) {
  MGPU_FORWARD_ERROR(validate_shader_program_stages(create_info));

  mgpu::Result<mgpu::ShaderProgramBase*> cxx_shader_program_result = ((mgpu::DeviceBase*)device)->CreateShaderProgram(*create_info);
  MGPU_FORWARD_ERROR(cxx_shader_program_result.Code());
  *shader_program = (MGPUShaderProgram)cxx_shader_program_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateRasterizerState(MGPUDevice device, const MGPURasterizerStateCreateInfo* create_info, MGPURasterizerState* rasterizer_state) {
  // TODO(fleroviux): implement validation?
  mgpu::Result<mgpu::RasterizerStateBase*> cxx_rasterizer_state_result = ((mgpu::DeviceBase*)device)->CreateRasterizerState(*create_info);
  MGPU_FORWARD_ERROR(cxx_rasterizer_state_result.Code());
  *rasterizer_state = (MGPURasterizerState)cxx_rasterizer_state_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateInputAssemblyState(MGPUDevice device, const MGPUInputAssemblyStateCreateInfo* create_info, MGPUInputAssemblyState* input_assembly_state) {
  // TODO(fleroviux): implement validation?
  mgpu::Result<mgpu::InputAssemblyStateBase*> cxx_input_assembly_state_result = ((mgpu::DeviceBase*)device)->CreateInputAssemblyState(*create_info);
  MGPU_FORWARD_ERROR(cxx_input_assembly_state_result.Code());
  *input_assembly_state = (MGPUInputAssemblyState)cxx_input_assembly_state_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateColorBlendState(MGPUDevice device, const MGPUColorBlendStateCreateInfo* create_info, MGPUColorBlendState* color_blend_state) {
  // TODO(fleroviux): implement validation?
  mgpu::Result<mgpu::ColorBlendStateBase*> cxx_color_blend_state_result = ((mgpu::DeviceBase*)device)->CreateColorBlendState(*create_info);
  MGPU_FORWARD_ERROR(cxx_color_blend_state_result.Code());
  *color_blend_state = (MGPUColorBlendState)cxx_color_blend_state_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateVertexInputState(MGPUDevice device, const MGPUVertexInputStateCreateInfo* create_info, MGPUVertexInputState* vertex_input_state) {
  // TODO(fleroviux): implement validation?
  // For example: that the number of bindings and attributes is within the device limits.
  mgpu::Result<mgpu::VertexInputStateBase*> cxx_vertex_input_state_result = ((mgpu::DeviceBase*)device)->CreateVertexInputState(*create_info);
  MGPU_FORWARD_ERROR(cxx_vertex_input_state_result.Code());
  *vertex_input_state = (MGPUVertexInputState)cxx_vertex_input_state_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateDepthStencilState(MGPUDevice device, const MGPUDepthStencilStateCreateInfo* create_info, MGPUDepthStencilState* depth_stencil_state) {
  // TODO(fleroviux): implement validation?
  mgpu::Result<mgpu::DepthStencilStateBase*> cxx_depth_stencil_state_result = ((mgpu::DeviceBase*)device)->CreateDepthStencilState(*create_info);
  MGPU_FORWARD_ERROR(cxx_depth_stencil_state_result.Code());
  *depth_stencil_state = (MGPUDepthStencilState)cxx_depth_stencil_state_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateCommandList(MGPUDevice device, MGPUCommandList* command_list) {
  const auto cxx_command_list = new(std::nothrow) mgpu::CommandList{};

  if(cxx_command_list == nullptr) {
    return MGPU_OUT_OF_MEMORY;
  }

  *command_list = (MGPUCommandList)cxx_command_list;
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateSwapChain(MGPUDevice device, const MGPUSwapChainCreateInfo* create_info, MGPUSwapChain* swap_chain) {
  // TODO(fleroviux): implement input validation
  mgpu::Result<mgpu::SwapChainBase*> cxx_swap_chain_result = ((mgpu::DeviceBase*)device)->CreateSwapChain(*create_info);
  MGPU_FORWARD_ERROR(cxx_swap_chain_result.Code());
  *swap_chain = (MGPUSwapChain)cxx_swap_chain_result.Unwrap();
  return MGPU_SUCCESS;
}

void mgpuDeviceDestroy(MGPUDevice device) {
  delete (mgpu::DeviceBase*)device;
}

}  // extern "C"
