
#pragma once

#include <mgpu/mgpu.h>
#include <atom/integer.hpp>

#include "backend/texture.hpp"
#include "common/texture.hpp"

inline MGPUResult validate_texture_format(MGPUTextureFormat texture_format) {
  // TODO(fleroviux): MSVC generates suboptimal code for this (GCC and Clang emit a single comparison)
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return MGPU_SUCCESS;
  }
  return MGPU_BAD_ENUM;
}

inline MGPUResult validate_texture_formats_compatible(MGPUTextureFormat texture_format_a, MGPUTextureFormat texture_format_b) {
  if(MGPUTextureFormatsCompatible(texture_format_a, texture_format_b)) {
    return MGPU_SUCCESS;
  }
  return MGPU_INCOMPATIBLE_TEXTURE_FORMAT;
}

inline MGPUResult validate_texture_format_has_aspect(MGPUTextureFormat texture_format, MGPUTextureAspect texture_aspect) {
  if((texture_aspect & ~MGPUTextureFormatToMGPUTextureAspect(texture_format)) != 0) {
    return MGPU_INCOMPATIBLE_TEXTURE_ASPECT;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_texture_type(MGPUTextureType texture_type) {
  // TODO(fleroviux): MSVC generates suboptimal code for this (GCC and Clang emit a single comparison)
  switch(texture_type) {
    case MGPU_TEXTURE_TYPE_1D:
    case MGPU_TEXTURE_TYPE_2D:
    case MGPU_TEXTURE_TYPE_3D:
      return MGPU_SUCCESS;
  }
  return MGPU_BAD_ENUM;
}

inline MGPUResult validate_texture_usage(MGPUTextureUsage texture_usage) {
  if(texture_usage == 0) {
    // TODO(fleroviux): reconsider what result code this error should return.
    return MGPU_BAD_ENUM;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_texture_aspect(MGPUTextureAspect texture_aspect) {
  if(texture_aspect == 0u) {
    // TODO(fleroviux): figure out if there a more meaningful error we should signal.
    return MGPU_BAD_ENUM;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_texture_extent(const MGPUPhysicalDeviceLimits& limits, MGPUTextureType texture_type, const MGPUExtent3D& texture_extent) {
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

inline MGPUResult validate_texture_mip_count(const MGPUExtent3D& texture_extent, u32 mip_count) {
  const u32 max_extent = std::max(texture_extent.width, std::max(texture_extent.height, texture_extent.depth));
  if(mip_count > (u32)std::log2(max_extent) + 1u) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_texture_array_layer_count(const MGPUPhysicalDeviceLimits& limits, u32 array_layer_count) {
  if(array_layer_count > limits.max_texture_array_layers) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_texture_contains_mip_range(mgpu::TextureBase* texture, u32 base_mip, u32 mip_count) {
  const u32 max_mip_level = base_mip + mip_count;
  if(max_mip_level <= base_mip || max_mip_level > texture->MipCount()) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

inline MGPUResult validate_texture_contains_array_layer_range(mgpu::TextureBase* texture, u32 base_array_layer, u32 array_layer_count) {
  const u32 max_array_layer = base_array_layer + array_layer_count;
  if(max_array_layer <= base_array_layer || max_array_layer > texture->ArrayLayerCount()) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}
