
#pragma once

#include <mgpu/mgpu.h>
#include <atom/integer.hpp>

#include "backend/buffer.hpp"

static inline MGPUResult validate_buffer_host_visible(mgpu::BufferBase* buffer) {
  if(!buffer->HostVisible()) {
    return MGPU_BUFFER_NOT_HOST_VISIBLE;
  }
  return MGPU_SUCCESS;
}

static inline MGPUResult validate_buffer_mapped(mgpu::BufferBase* buffer) {
  if(!buffer->IsMapped()) {
    return MGPU_BUFFER_NOT_MAPPED;
  }
  return MGPU_SUCCESS;
}
