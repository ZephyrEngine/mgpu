
#include <mgpu/mgpu.h>

#include "backend/buffer.hpp"

static MGPUResult validate_buffer_host_visible(mgpu::BufferBase* buffer) {
  if(!buffer->HostVisible()) {
    return MGPU_BUFFER_NOT_HOST_VISIBLE;
  }
  return MGPU_SUCCESS;
}

static MGPUResult validate_buffer_mapped(mgpu::BufferBase* buffer) {
  if(!buffer->IsMapped()) {
    return MGPU_BUFFER_NOT_MAPPED;
  }
  return MGPU_SUCCESS;
}

extern "C" {

MGPUResult mgpuBufferMap(MGPUBuffer buffer, void** address) {
  MGPU_FORWARD_ERROR(validate_buffer_host_visible((mgpu::BufferBase*)buffer));

  mgpu::Result<void*> address_result = ((mgpu::BufferBase*)buffer)->Map();
  MGPU_FORWARD_ERROR(address_result.Code());
  *address = address_result.Unwrap();
  return MGPU_SUCCESS;
}

MGPUResult mgpuBufferUnmap(MGPUBuffer buffer) {
  MGPU_FORWARD_ERROR(validate_buffer_host_visible((mgpu::BufferBase*)buffer));
  return ((mgpu::BufferBase*)buffer)->Unmap();
}

MGPUResult mgpuBufferFlushRange(MGPUBuffer buffer, uint64_t offset, uint64_t size) {
  auto cxx_buffer = (mgpu::BufferBase*)buffer;
  MGPU_FORWARD_ERROR(validate_buffer_host_visible(cxx_buffer));
  MGPU_FORWARD_ERROR(validate_buffer_mapped(cxx_buffer));

  const uint64_t buffer_size = cxx_buffer->Size();
  const uint64_t offset_plus_size = offset + size;
  if(offset_plus_size < offset || offset_plus_size >= buffer_size) {
    size = buffer_size - offset;
  }
  return cxx_buffer->FlushRange(offset, size);
}

void mgpuBufferDestroy(MGPUBuffer buffer) {
  delete (mgpu::BufferBase*)buffer;
}

}  // extern "C"
