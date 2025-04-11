
#pragma once

#include <mgpu/mgpu.h>
//#include <atom/integer.hpp>
#include <algorithm>

inline MGPUResult validate_sampler_mip_lod_bias(const MGPUPhysicalDeviceLimits& limits, float mip_lod_bias) {
  if(std::abs(mip_lod_bias) > limits.max_sampler_lod_bias) {
    // TODO(fleroviux): what's the proper return code to return here?
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_sampler_lod_clamp(float min_lod, float max_lod) {
  if(max_lod < min_lod) {
    // TODO(fleroviux): what's the proper return code to return here?
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_sampler_max_anisotropy(const MGPUPhysicalDeviceLimits& limits, float max_anisotropy) {
  if(max_anisotropy < 1.0f || max_anisotropy > limits.max_sampler_anisotropy) {
    // TODO(fleroviux): what's the proper return code to return here?
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

//#include "backend/buffer.hpp"
//
//inline MGPUResult validate_buffer_size(u64 buffer_size) {
//  if(buffer_size == 0u) {
//    return MGPU_BAD_DIMENSIONS;
//  }
//  return MGPU_SUCCESS;
//}
//
//inline MGPUResult validate_buffer_usage(MGPUBufferUsage buffer_usage) {
//  if(buffer_usage == 0) {
//    // TODO(fleroviux): reconsider what result code this error should return.
//    return MGPU_BAD_ENUM;
//  }
//  return MGPU_SUCCESS;
//}
//
//inline MGPUResult validate_buffer_host_visible(mgpu::BufferBase* buffer) {
//  if(!buffer->HostVisible()) {
//    return MGPU_BUFFER_NOT_HOST_VISIBLE;
//  }
//  return MGPU_SUCCESS;
//}
//
//inline MGPUResult validate_buffer_mapped(mgpu::BufferBase* buffer) {
//  if(!buffer->IsMapped()) {
//    return MGPU_BUFFER_NOT_MAPPED;
//  }
//  return MGPU_SUCCESS;
//}
//
//inline MGPUResult validate_buffer_range(mgpu::BufferBase* buffer, u64 offset, u64 size) {
//  const u64 buffer_size = buffer->Size();
//  const u64 high_offset = offset + size;
//
//  // NOTE: the 'offset == buffer_size' check is needed when the range size is zero.
//  if(high_offset < offset || high_offset > buffer_size || offset == buffer_size) {
//    return MGPU_BAD_DIMENSIONS;
//  }
//  return MGPU_SUCCESS;
//}
//
//inline MGPUResult validate_buffer_has_usage_bits(mgpu::BufferBase* buffer, MGPUBufferUsage usage_bits) {
//  if((buffer->Usage() & usage_bits) != usage_bits) {
//    return MGPU_BUFFER_INCOMPATIBLE;
//  }
//  return MGPU_SUCCESS;
//}