
#pragma once

#include <mgpu/mgpu.h>
#include <atom/integer.hpp>

#include "backend/buffer.hpp"

inline MGPUResult validate_buffer_size(u64 buffer_size) {
  if(buffer_size == 0u) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_buffer_usage(MGPUBufferUsage buffer_usage) {
  if(buffer_usage == 0) {
    // TODO(fleroviux): reconsider what result code this error should return.
    return MGPU_BAD_ENUM;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_buffer_host_visible(mgpu::BufferBase* buffer) {
  if(!buffer->HostVisible()) {
    return MGPU_BUFFER_NOT_HOST_VISIBLE;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_buffer_mapped(mgpu::BufferBase* buffer) {
  if(!buffer->IsMapped()) {
    return MGPU_BUFFER_NOT_MAPPED;
  }
  return MGPU_SUCCESS;
}
