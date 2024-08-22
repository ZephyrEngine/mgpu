
#include <mgpu/mgpu.h>

#include "backend/command_list.hpp"
#include "backend/device.hpp"
#include "validation/buffer.hpp"
#include "validation/texture.hpp"

extern "C" {

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
  MGPU_FORWARD_ERROR(validate_texture_extent(cxx_device->Limits(), create_info->type, create_info->extent));
  MGPU_FORWARD_ERROR(validate_texture_mip_count(create_info->extent, create_info->mip_count));
  MGPU_FORWARD_ERROR(validate_texture_array_layer_count(cxx_device->Limits(), create_info->array_layer_count));

  mgpu::Result<mgpu::TextureBase*> cxx_texture_result = cxx_device->CreateTexture(*create_info);
  MGPU_FORWARD_ERROR(cxx_texture_result.Code());
  *texture = (MGPUTexture)cxx_texture_result.Unwrap();
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

MGPUResult mgpuDeviceSubmitCommandList(MGPUDevice device, MGPUCommandList command_list) {
  return ((mgpu::DeviceBase*)device)->SubmitCommandList((const mgpu::CommandList*)command_list);
}

MGPUResult mgpuDeviceFlush(MGPUDevice device) {
  return ((mgpu::DeviceBase*)device)->Flush();
}

void mgpuDeviceDestroy(MGPUDevice device) {
  delete (mgpu::DeviceBase*)device;
}

}  // extern "C"
