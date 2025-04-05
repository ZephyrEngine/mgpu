
#pragma once

#include <mgpu/mgpu.h>

#include <atom/integer.hpp>
#include <atom/panic.hpp>

inline MGPUTextureAspect MGPUTextureFormatToMGPUTextureAspect(MGPUTextureFormat texture_format) {
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return MGPU_TEXTURE_ASPECT_COLOR;
    case MGPU_TEXTURE_FORMAT_DEPTH_F32: return MGPU_TEXTURE_ASPECT_DEPTH;
    default: ATOM_PANIC("unhandled texture format: {}", (int)texture_format);
  }
}

inline bool MGPUTextureFormatIsCompressed(MGPUTextureFormat texture_format) {
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB:
    case MGPU_TEXTURE_FORMAT_DEPTH_F32:
      return false;
    default: ATOM_PANIC("unhandled texture format: {}", (int)texture_format);
  }
}

inline bool MGPUTextureFormatIsDepthStencil(MGPUTextureFormat texture_format) {
  // TODO(fleroviux): this possibly could be implemented via MGPUTextureFormatToMGPUTextureAspect()
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return false;
    case MGPU_TEXTURE_FORMAT_DEPTH_F32: return true;
    default: ATOM_PANIC("unhandled texture format: {}", (int)texture_format);
  }
}

inline size_t MGPUTextureFormatGetTexelSize(MGPUTextureFormat texture_format) {
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB:
    case MGPU_TEXTURE_FORMAT_DEPTH_F32:
      return sizeof(u32);
    default: ATOM_PANIC("unhandled texture format: {}", (int)texture_format);
  }
}

inline bool MGPUTextureFormatHasAlpha(MGPUTextureFormat texture_format) {
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return true;
    case MGPU_TEXTURE_FORMAT_DEPTH_F32: return false;
    default: ATOM_PANIC("unhandled texture format: {}", (int)texture_format);
  }
}

inline bool MGPUTextureFormatsCompatible(MGPUTextureFormat texture_format_a, MGPUTextureFormat texture_format_b) {
  // We follow Vulkan's rules for texture format compatibility here:
  // https://registry.khronos.org/vulkan/specs/1.2-extensions/html/vkspec.html#formats-compatibility-classes

  if(texture_format_a == texture_format_b) {
    return true;
  }

  // TODO(fleroviux): handle compressed texture formats, once we support them.
  if(MGPUTextureFormatIsCompressed(texture_format_a) || MGPUTextureFormatIsCompressed(texture_format_b)) {
    ATOM_PANIC("unhandled compressed texture format");
  }

  if(MGPUTextureFormatIsDepthStencil(texture_format_a) || MGPUTextureFormatIsDepthStencil(texture_format_b)) {
    return false; // Depth/Stencil formats are only compatible with themselves.
  }

  if(MGPUTextureFormatGetTexelSize(texture_format_a) != MGPUTextureFormatGetTexelSize(texture_format_b)) {
    return false; // Uncompressed color formats can only be compatible if they have equal bits per texel block.
  }

  if(MGPUTextureFormatHasAlpha(texture_format_a) != MGPUTextureFormatHasAlpha(texture_format_b)) {
    return false; // Uncompressed color formats can only be compatible if either both or neither have an alpha channel.
  }

  return true;
}