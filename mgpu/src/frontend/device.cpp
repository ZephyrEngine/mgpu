
#include <mgpu/mgpu.h>

#include "backend/device.hpp"

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

void mgpuDeviceDestroy(MGPUDevice device) {
  delete (mgpu::DeviceBase*)device;
}

}  // extern "C"
