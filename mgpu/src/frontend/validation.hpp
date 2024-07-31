
#pragma once

#include <mgpu/mgpu.h>
#include <algorithm>
#include <atom/integer.hpp>
#include <cmath>

#include "backend/buffer.hpp"
#include "backend/texture.hpp"
#include "common/texture.hpp"

// Buffer validation

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

// Texture validation

static MGPUResult validate_texture_format(MGPUTextureFormat texture_format) {
  // TODO(fleroviux): MSVC generates suboptimal code for this (GCC and Clang emit a single comparison)
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return MGPU_SUCCESS;
  }
  return MGPU_BAD_ENUM;
}

static MGPUResult validate_texture_formats_compatible(MGPUTextureFormat texture_format_a, MGPUTextureFormat texture_format_b) {
  if(MGPUTextureFormatsCompatible(texture_format_a, texture_format_b)) {
    return MGPU_SUCCESS;
  }
  return MGPU_INCOMPATIBLE_TEXTURE_FORMAT;
}

static MGPUResult validate_texture_format_has_aspect(MGPUTextureFormat texture_format, MGPUTextureAspect texture_aspect) {
  if((texture_aspect & ~MGPUTextureFormatToMGPUTextureAspect(texture_format)) != 0) {
    return MGPU_INCOMPATIBLE_TEXTURE_ASPECT;
  }
  return MGPU_SUCCESS;
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

static MGPUResult validate_texture_aspect(MGPUTextureAspect texture_aspect) {
  if(texture_aspect == 0u) {
    // TODO(fleroviux): figure out if there a more meaningful error we should signal.
    return MGPU_BAD_ENUM;
  }
  return MGPU_SUCCESS;
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

static MGPUResult validate_texture_mip_count(const MGPUPhysicalDeviceLimits& limits, const MGPUExtent3D& texture_extent, u32 mip_count) {
  const u32 max_extent = std::max(texture_extent.width, std::max(texture_extent.height, texture_extent.depth));
  if(mip_count > (u32)std::log2(max_extent) + 1u) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

static MGPUResult validate_texture_array_layer_count(const MGPUPhysicalDeviceLimits& limits, u32 array_layer_count) {
  if(array_layer_count > limits.max_texture_array_layers) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

static MGPUResult validate_texture_contains_mip_range(mgpu::TextureBase* texture, u32 base_mip, u32 mip_count) {
  const u32 max_mip_level = base_mip + mip_count;
  if(max_mip_level <= base_mip || max_mip_level > texture->MipCount()) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

static MGPUResult validate_texture_contains_array_layer_range(mgpu::TextureBase* texture, u32 base_array_layer, u32 array_layer_count) {
  const u32 max_array_layer = base_array_layer + array_layer_count;
  if(max_array_layer <= base_array_layer || max_array_layer > texture->ArrayLayerCount()) {
    return MGPU_BAD_DIMENSIONS;
  }
  return MGPU_SUCCESS;
}

// Texture view validation

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

static MGPUResult validate_texture_supports_view_type(MGPUTextureViewType texture_view_type, mgpu::TextureBase* texture) {
  const MGPUTextureType texture_type = texture->Type();

  switch(texture_view_type) {
    case MGPU_TEXTURE_VIEW_TYPE_1D:
    case MGPU_TEXTURE_VIEW_TYPE_1D_ARRAY: {
      if(texture_type != MGPU_TEXTURE_TYPE_1D) {
        return MGPU_INCOMPATIBLE_TEXTURE_VIEW_TYPE;
      }
      break;
    }
    case MGPU_TEXTURE_VIEW_TYPE_2D:
    case MGPU_TEXTURE_VIEW_TYPE_2D_ARRAY: {
      if(texture_type == MGPU_TEXTURE_TYPE_1D) {
        return MGPU_INCOMPATIBLE_TEXTURE_VIEW_TYPE;
      }
      break;
    }
    case MGPU_TEXTURE_VIEW_TYPE_CUBE:
    case MGPU_TEXTURE_VIEW_TYPE_CUBE_ARRAY: {
      if(texture_type != MGPU_TEXTURE_TYPE_2D || !texture->IsCubeCompatible()) {
        return MGPU_INCOMPATIBLE_TEXTURE_VIEW_TYPE;
      }
      break;
    }
    case MGPU_TEXTURE_VIEW_TYPE_3D: {
      if(texture_type != MGPU_TEXTURE_TYPE_3D) {
        return MGPU_INCOMPATIBLE_TEXTURE_VIEW_TYPE;
      }
      break;
    }
  }

  return MGPU_SUCCESS;
}

static MGPUResult validate_texture_view_mip_range(mgpu::TextureBase* texture, MGPUTextureViewType texture_view_type, u32 base_mip, u32 mip_count) {
  MGPU_FORWARD_ERROR(validate_texture_contains_mip_range(texture, base_mip, mip_count));

  switch(texture_view_type) {
    case MGPU_TEXTURE_VIEW_TYPE_2D:
    case MGPU_TEXTURE_VIEW_TYPE_2D_ARRAY: {
      if(texture->Type() == MGPU_TEXTURE_TYPE_3D && mip_count != 1u) {
        return MGPU_BAD_DIMENSIONS;
      }
      break;
    }
    default: break;
  }

  return MGPU_SUCCESS;
}

static MGPUResult validate_texture_view_array_layer_range(mgpu::TextureBase* texture, MGPUTextureViewType texture_view_type, u32 base_array_layer, u32 array_layer_count) {
  MGPU_FORWARD_ERROR(validate_texture_contains_array_layer_range(texture, base_array_layer, array_layer_count));

  switch(texture_view_type) {
    case MGPU_TEXTURE_VIEW_TYPE_1D:
    case MGPU_TEXTURE_VIEW_TYPE_2D:
    case MGPU_TEXTURE_VIEW_TYPE_3D: {
      if(array_layer_count != 1u) {
        return MGPU_BAD_DIMENSIONS;
      }
      break;
    }
    case MGPU_TEXTURE_VIEW_TYPE_CUBE: {
      if(array_layer_count != 6u) {
        return MGPU_BAD_DIMENSIONS;
      }
      break;
    }
    case MGPU_TEXTURE_VIEW_TYPE_CUBE_ARRAY: {
      if(array_layer_count == 0u || array_layer_count % 6u != 0u) {
        return MGPU_BAD_DIMENSIONS;
      }
      break;
    }
    default: break;
  }

  return MGPU_SUCCESS;
}
