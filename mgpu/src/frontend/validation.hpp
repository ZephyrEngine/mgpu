
#pragma once

#include <mgpu/mgpu.h>
#include <algorithm>
#include <atom/integer.hpp>
#include <cmath>

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

static MGPUResult validate_texture_format(MGPUTextureFormat texture_format) {
  // TODO(fleroviux): MSVC generates suboptimal code for this (GCC and Clang emit a single comparison)
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return MGPU_SUCCESS;
  }
  return MGPU_BAD_ENUM;
}

static MGPUResult validate_texture_type(MGPUTextureType texture_type) {
  // TODO(fleroviux): MSVC generates suboptimal code for this (GCC and Clang emit a single comparison)
  switch(texture_type) {
    case MGPU_TEXTURE_TYPE_1D:
    case MGPU_TEXTURE_TYPE_2D:
    case MGPU_TEXTURE_TYPE_3D:
      return MGPU_SUCCESS;
  }
  return MGPU_BAD_ENUM;
}

static MGPUResult validate_texture_extent(const MGPUPhysicalDeviceLimits& limits, MGPUTextureType texture_type, const MGPUExtent3D& texture_extent) {
  const u32 width = texture_extent.width;
  const u32 height = texture_extent.height;
  const u32 depth = texture_extent.depth;

  // The Vulkan spec demands that for 1D textures height and depth are set to one and for 2D textures depth is set to one.
  switch(texture_type) {
    case MGPU_TEXTURE_TYPE_1D: {
      if(width == 0u || height != 1u || depth != 1u || width > limits.max_texture_dimension_1d) {
        return MGPU_BAD_DIMENSIONS;
      }
      return MGPU_SUCCESS;
    }
    case MGPU_TEXTURE_TYPE_2D: {
      const u32 limit = limits.max_texture_dimension_2d;
      if(width == 0u || height == 0u || depth != 1u || width > limit || height > limit) {
        return MGPU_BAD_DIMENSIONS;
      }
      return MGPU_SUCCESS;
    }
    case MGPU_TEXTURE_TYPE_3D: {
      const u32 limit = limits.max_texture_dimension_3d;
      if(width == 0u || height == 0u || depth == 0u || width > limit || height > limit || depth > limit) {
        return MGPU_BAD_DIMENSIONS;
      }
      return MGPU_SUCCESS;
    }
  }
  return MGPU_BAD_ENUM;
}

static MGPUResult validate_texture_array_layer_count(const MGPUPhysicalDeviceLimits& limits, u32 array_layer_count) {
  if(array_layer_count > limits.max_texture_array_layers) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

static MGPUResult validate_texture_mip_count(const MGPUPhysicalDeviceLimits& limits, const MGPUExtent3D& texture_extent, u32 mip_count) {
  const u32 max_extent = std::max(texture_extent.width, std::max(texture_extent.height, texture_extent.depth));
  if(mip_count > (u32)std::log2(max_extent) + 1u) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

static MGPUResult validate_texture_view_type(MGPUTextureViewType texture_view_type) {
  // TODO(fleroviux): MSVC generates suboptimal code for this (GCC and Clang emit a single comparison)
  switch(texture_view_type) {
    case MGPU_TEXTURE_VIEW_TYPE_1D:
    case MGPU_TEXTURE_VIEW_TYPE_2D:
    case MGPU_TEXTURE_VIEW_TYPE_3D:
    case MGPU_TEXTURE_VIEW_TYPE_CUBE:
    case MGPU_TEXTURE_VIEW_TYPE_1D_ARRAY:
    case MGPU_TEXTURE_VIEW_TYPE_2D_ARRAY:
    case MGPU_TEXTURE_VIEW_TYPE_CUBE_ARRAY:
      return MGPU_SUCCESS;
  }
  return MGPU_BAD_ENUM;
}