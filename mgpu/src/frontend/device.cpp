
#include <mgpu/mgpu.h>

#include "backend/command_list.hpp"
#include "backend/device.hpp"
#include "validation/buffer.hpp"
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
  mgpu::DeviceBase* cxx_device = (mgpu::DeviceBase*)device;

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

MGPUResult mgpuDeviceCreateShaderModule(MGPUDevice device, const uint32_t* spirv_code, size_t spirv_byte_size, MGPUShaderModule* shader_module) {
  mgpu::Result<mgpu::ShaderModuleBase*> cxx_shader_module_result = ((mgpu::DeviceBase*)device)->CreateShaderModule(spirv_code, spirv_byte_size);
  MGPU_FORWARD_ERROR(cxx_shader_module_result.Code());
  *shader_module = (MGPUShaderModule)cxx_shader_module_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateShaderProgram(MGPUDevice device, const MGPUShaderProgramCreateInfo* create_info, MGPUShaderProgram* shader_program) {
  // TODO(fleroviux): validate that the pipeline stage combination is valid.
  mgpu::Result<mgpu::ShaderProgramBase*> cxx_shader_program_result = ((mgpu::DeviceBase*)device)->CreateShaderProgram(*create_info);
  MGPU_FORWARD_ERROR(cxx_shader_program_result.Code());
  *shader_program = (MGPUShaderProgram)cxx_shader_program_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateCommandList(MGPUDevice device, MGPUCommandList* command_list) {
  mgpu::CommandList* cxx_command_list = new(std::nothrow) mgpu::CommandList{};

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
