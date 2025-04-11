
#pragma once

#include "texture.hpp"

// TODO(fleroviux): we only really do this for textures and texture views as far as I can tell, so it's really inconsistent.
inline MGPUResult validate_texture_view_type(MGPUTextureViewType texture_view_type) {
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

inline MGPUResult validate_texture_supports_view_type(MGPUTextureViewType texture_view_type, mgpu::TextureBase* texture) {
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

inline MGPUResult validate_texture_view_mip_range(mgpu::TextureBase* texture, MGPUTextureViewType texture_view_type, u32 base_mip, u32 mip_count) {
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

inline MGPUResult validate_texture_view_array_layer_range(mgpu::TextureBase* texture, MGPUTextureViewType texture_view_type, u32 base_array_layer, u32 array_layer_count) {
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
