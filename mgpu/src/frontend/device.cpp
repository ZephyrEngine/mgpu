
#include <mgpu/mgpu.h>

#include "backend/device.hpp"
#include "validation.hpp"

extern "C" {

MGPUResult mgpuDeviceCreateBuffer(MGPUDevice device, const MGPUBufferCreateInfo* create_info, MGPUBuffer* buffer) {
  if(create_info->size == 0u) {
    return MGPU_BAD_DIMENSIONS;
  }

  if(create_info->usage == 0) {
    // TODO(fleroviux): reconsider what result code this error should return.
    return MGPU_BAD_ENUM;
  }

  mgpu::Result<mgpu::BufferBase*> cxx_buffer_result = ((mgpu::DeviceBase*)device)->CreateBuffer(*create_info);
  MGPU_FORWARD_ERROR(cxx_buffer_result.Code());
  *buffer = (MGPUBuffer)cxx_buffer_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuDeviceCreateTexture(MGPUDevice device, const MGPUTextureCreateInfo* create_info, MGPUTexture* texture) {
  mgpu::DeviceBase* cxx_device = (mgpu::DeviceBase*)device;

  MGPU_FORWARD_ERROR(validate_texture_format(create_info->format));
  MGPU_FORWARD_ERROR(validate_texture_type(create_info->type));
  MGPU_FORWARD_ERROR(validate_texture_extent(cxx_device->Limits(), create_info->type, create_info->extent));
  MGPU_FORWARD_ERROR(validate_texture_array_layer_count(cxx_device->Limits(), create_info->array_layer_count));
  MGPU_FORWARD_ERROR(validate_texture_mip_count(cxx_device->Limits(), create_info->extent, create_info->mip_count));

  if(create_info->usage == 0) {
    // TODO(fleroviux): reconsider what result code this error should return.
    return MGPU_BAD_ENUM;
  }

  mgpu::Result<mgpu::TextureBase*> cxx_texture_result = cxx_device->CreateTexture(*create_info);
  MGPU_FORWARD_ERROR(cxx_texture_result.Code());
  *texture = (MGPUTexture)cxx_texture_result.Unwrap();
  return MGPU_SUCCESS;
}

void mgpuDeviceDestroy(MGPUDevice device) {
  delete (mgpu::DeviceBase*)device;
}

}  // extern "C"
