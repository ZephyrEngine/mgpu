
#pragma once

#include <mgpu/mgpu.h>

#include <atom/panic.hpp>
#include <vulkan/vulkan.h>

#include "common/texture.hpp"

namespace mgpu::vulkan {

inline VkExtent3D MGPUExtent3DToVkExtent3D(MGPUExtent3D extent) {
  return {.width = extent.width, .height = extent.height, .depth = extent.depth};
}

inline VkOffset3D MGPUOffset3DToVkOffset3D(MGPUOffset3D offset) {
  return {.x = offset.x, .y = offset.y, .z = offset.z};
}

inline VkBufferUsageFlags MGPUBufferUsageToVkBufferUsage(MGPUBufferUsage buffer_usage) {
  VkBufferUsageFlags vk_buffer_usage{};
  if(buffer_usage & MGPU_BUFFER_USAGE_COPY_SRC)        vk_buffer_usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_COPY_DST)        vk_buffer_usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_UNIFORM_BUFFER)  vk_buffer_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_STORAGE_BUFFER)  vk_buffer_usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_INDEX_BUFFER)    vk_buffer_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_VERTEX_BUFFER)   vk_buffer_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if(buffer_usage & MGPU_BUFFER_USAGE_INDIRECT_BUFFER) vk_buffer_usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  return vk_buffer_usage;
}

inline VkImageType MGPUTextureTypeToVkImageType(MGPUTextureType texture_type) {
  switch(texture_type) {
    case MGPU_TEXTURE_TYPE_1D: return VK_IMAGE_TYPE_1D;
    case MGPU_TEXTURE_TYPE_2D: return VK_IMAGE_TYPE_2D;
    case MGPU_TEXTURE_TYPE_3D: return VK_IMAGE_TYPE_3D;
    default: ATOM_PANIC("unhandled texture type: {}", (int)texture_type);
  }
}

inline VkFormat MGPUTextureFormatToVkFormat(MGPUTextureFormat texture_format) {
  switch(texture_format) {
    case MGPU_TEXTURE_FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
    case MGPU_TEXTURE_FORMAT_DEPTH_F32: return VK_FORMAT_D32_SFLOAT;
    default: ATOM_PANIC("unhandled texture format: {}", (int)texture_format);
  }
}

inline VkImageUsageFlags MGPUTextureUsageToVkImageUsage(MGPUTextureFormat texture_format, MGPUTextureUsage texture_usage) {
  VkImageUsageFlags vk_image_usage{};
  if(texture_usage & MGPU_TEXTURE_USAGE_COPY_SRC) vk_image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_COPY_DST) vk_image_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_SAMPLED)  vk_image_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_STORAGE)  vk_image_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
  if(texture_usage & MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT) {
    if(MGPUTextureFormatToMGPUTextureAspect(texture_format) & MGPU_TEXTURE_ASPECT_COLOR) {
      vk_image_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    } else {
      // TODO(fleroviux): validate that this always correct.
      vk_image_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
  }
  return vk_image_usage;
}

inline VkImageUsageFlags VkImageUsageToMGPUTextureUsage(VkImageUsageFlags image_usage) {
  MGPUTextureUsage mgpu_texture_usage{};
  if(image_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) mgpu_texture_usage |= MGPU_TEXTURE_USAGE_COPY_SRC;
  if(image_usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) mgpu_texture_usage |= MGPU_TEXTURE_USAGE_COPY_DST;
  if(image_usage & VK_IMAGE_USAGE_SAMPLED_BIT)      mgpu_texture_usage |= MGPU_TEXTURE_USAGE_SAMPLED;
  if(image_usage & VK_IMAGE_USAGE_STORAGE_BIT)      mgpu_texture_usage |= MGPU_TEXTURE_USAGE_STORAGE;
  if(image_usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) mgpu_texture_usage |= MGPU_TEXTURE_USAGE_RENDER_ATTACHMENT;
  return mgpu_texture_usage;
}

inline VkImageAspectFlags MGPUTextureAspectToVkImageAspect(MGPUTextureAspect texture_aspect) {
  VkImageAspectFlags vk_image_aspect{};
  if(texture_aspect & MGPU_TEXTURE_ASPECT_COLOR)   vk_image_aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
  if(texture_aspect & MGPU_TEXTURE_ASPECT_DEPTH)   vk_image_aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
  if(texture_aspect & MGPU_TEXTURE_ASPECT_STENCIL) vk_image_aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
  return vk_image_aspect;
}

inline VkImageViewType MGPUTextureViewTypeToVkImageViewType(MGPUTextureViewType texture_view_type) {
  switch(texture_view_type) {
    case MGPU_TEXTURE_VIEW_TYPE_1D:         return VK_IMAGE_VIEW_TYPE_1D;
    case MGPU_TEXTURE_VIEW_TYPE_2D:         return VK_IMAGE_VIEW_TYPE_2D;
    case MGPU_TEXTURE_VIEW_TYPE_3D:         return VK_IMAGE_VIEW_TYPE_3D;
    case MGPU_TEXTURE_VIEW_TYPE_CUBE:       return VK_IMAGE_VIEW_TYPE_CUBE;
    case MGPU_TEXTURE_VIEW_TYPE_1D_ARRAY:   return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case MGPU_TEXTURE_VIEW_TYPE_2D_ARRAY:   return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case MGPU_TEXTURE_VIEW_TYPE_CUBE_ARRAY: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    default: ATOM_PANIC("unhandled texture view type: {}", (int)texture_view_type);
  }
}

inline VkFilter MGPUTextureFilterToVkFilter(MGPUTextureFilter texture_filter) {
  switch(texture_filter) {
    case MGPU_TEXTURE_FILTER_NEAREST: return VK_FILTER_NEAREST;
    case MGPU_TEXTURE_FILTER_LINEAR:  return VK_FILTER_LINEAR;
    default: ATOM_PANIC("unhandled texture filter: {}", (int)texture_filter);
  }
}

inline VkSamplerMipmapMode MGPUTextureFilterToVkSamplerMipmapMode(MGPUTextureFilter texture_filter) {
  switch(texture_filter) {
    case MGPU_TEXTURE_FILTER_NEAREST: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case MGPU_TEXTURE_FILTER_LINEAR:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default: ATOM_PANIC("unhandled texture filter: {}", (int)texture_filter);
  }
}

inline VkSamplerAddressMode MGPUSamplerAddressModeToVkSamplerAddressMode(MGPUSamplerAddressMode address_mode) {
  switch(address_mode) {
    case MGPU_SAMPLER_ADDRESS_MODE_REPEAT:          return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case MGPU_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case MGPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:   return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    default: ATOM_PANIC("unhandled sampler address mode: {}", (int)address_mode);
  }
}

inline VkShaderStageFlagBits MGPUShaderStageBitToVkShaderStageBit(MGPUShaderStageBits shader_stage_bit) {
  switch(shader_stage_bit) {
    case MGPU_SHADER_STAGE_VERTEX:                  return VK_SHADER_STAGE_VERTEX_BIT;
    case MGPU_SHADER_STAGE_TESSELLATION_CONTROL:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case MGPU_SHADER_STAGE_TESSELLATION_EVALUATION: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case MGPU_SHADER_STAGE_GEOMETRY:                return VK_SHADER_STAGE_GEOMETRY_BIT;
    case MGPU_SHADER_STAGE_FRAGMENT:                return VK_SHADER_STAGE_FRAGMENT_BIT;
    case MGPU_SHADER_STAGE_COMPUTE:                 return VK_SHADER_STAGE_COMPUTE_BIT;
    default: ATOM_PANIC("unhandled shader stage bit: {}", (int)shader_stage_bit);
  }
}

inline VkShaderStageFlags MGPUShaderStagesToVkShaderStageFlags(MGPUShaderStage shader_stages) {
  VkShaderStageFlags vk_shader_stage_flags = 0;

  while(shader_stages != 0) {
    const auto shader_stage_bit = (MGPUShaderStageBits)(shader_stages & ~(shader_stages - 1));
    vk_shader_stage_flags |= MGPUShaderStageBitToVkShaderStageBit(shader_stage_bit);
    shader_stages &= ~shader_stage_bit;
  }
  return vk_shader_stage_flags;
}

inline VkDescriptorType MGPUResourceBindingTypeToVkDescriptorType(MGPUResourceBindingType resource_binding_type) {
  switch(resource_binding_type) {
    case MGPU_RESOURCE_BINDING_TYPE_SAMPLER:             return VK_DESCRIPTOR_TYPE_SAMPLER;
    case MGPU_RESOURCE_BINDING_TYPE_TEXTURE_AND_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case MGPU_RESOURCE_BINDING_TYPE_SAMPLED_TEXTURE:     return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case MGPU_RESOURCE_BINDING_TYPE_STORAGE_TEXTURE:     return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case MGPU_RESOURCE_BINDING_TYPE_UNIFORM_BUFFER:      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case MGPU_RESOURCE_BINDING_TYPE_STORAGE_BUFFER:      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    default: ATOM_PANIC("unhandled resource binding type: {}", (int)resource_binding_type);
  }
}

inline VkPolygonMode MGPUPolygonModeToVkPolygonMode(MGPUPolygonMode polygon_mode) {
  switch(polygon_mode) {
    case MGPU_POLYGON_MODE_FILL:  return VK_POLYGON_MODE_FILL;
    case MGPU_POLYGON_MODE_LINE:  return VK_POLYGON_MODE_LINE;
    default: ATOM_PANIC("unhandled polygon mode: {}", (int)polygon_mode);
  }
}

inline VkCullModeFlags MGPUCullModeToVkCullMode(MGPUCullMode cull_mode) {
  VkCullModeFlags vk_cull_mode{};
  if(cull_mode & MGPU_CULL_MODE_FRONT) vk_cull_mode |= VK_CULL_MODE_FRONT_BIT;
  if(cull_mode & MGPU_CULL_MODE_BACK)  vk_cull_mode |= VK_CULL_MODE_BACK_BIT;
  return vk_cull_mode;
}

inline VkFrontFace MGPUFrontFaceToVkFrontFace(MGPUFrontFace front_face) {
  switch(front_face) {
    case MGPU_FRONT_FACE_CLOCKWISE:         return VK_FRONT_FACE_CLOCKWISE;
    case MGPU_FRONT_FACE_COUNTER_CLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    default: ATOM_PANIC("unhandled front face: {}", (int)front_face);
  }
}

inline VkPrimitiveTopology MGPUPrimitiveTopologyToVkPrimitiveTopology(MGPUPrimitiveTopology topology) {
  switch(topology) {
    case MGPU_PRIMITIVE_TOPOLOGY_POINT_LIST:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case MGPU_PRIMITIVE_TOPOLOGY_LINE_LIST:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case MGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case MGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case MGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case MGPU_PRIMITIVE_TOPOLOGY_PATCH_LIST:     return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default: ATOM_PANIC("unhandled primitive topology: {}", (int)topology);
  }
}

inline VkBlendFactor MGPUBlendFactorToVkBlendFactor(MGPUBlendFactor blend_factor) {
  switch(blend_factor) {
    case MGPU_BLEND_FACTOR_ZERO:                 return VK_BLEND_FACTOR_ZERO;
    case MGPU_BLEND_FACTOR_ONE:                  return VK_BLEND_FACTOR_ONE;
    case MGPU_BLEND_FACTOR_SRC_COLOR:            return VK_BLEND_FACTOR_SRC_COLOR;
    case MGPU_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:  return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case MGPU_BLEND_FACTOR_DST_COLOR:            return VK_BLEND_FACTOR_DST_COLOR;
    case MGPU_BLEND_FACTOR_ONE_MINUS_DST_COLOR:  return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case MGPU_BLEND_FACTOR_SRC_ALPHA:            return VK_BLEND_FACTOR_SRC_ALPHA;
    case MGPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:  return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case MGPU_BLEND_FACTOR_DST_ALPHA:            return VK_BLEND_FACTOR_DST_ALPHA;
    case MGPU_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:  return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case MGPU_BLEND_FACTOR_SRC_ALPHA_SATURATE:   return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case MGPU_BLEND_FACTOR_SRC1_COLOR:           return VK_BLEND_FACTOR_SRC1_COLOR;
    case MGPU_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case MGPU_BLEND_FACTOR_SRC1_ALPHA:           return VK_BLEND_FACTOR_SRC1_ALPHA;
    case MGPU_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default: ATOM_PANIC("unhandled blend factor: {}", (int)blend_factor);
  }
}

inline VkBlendOp MGPUBlendOpToVkBlendOp(MGPUBlendOp blend_op) {
  switch(blend_op) {
    case MGPU_BLEND_OP_ADD:              return VK_BLEND_OP_ADD;
    case MGPU_BLEND_OP_SUBTRACT:         return VK_BLEND_OP_SUBTRACT;
    case MGPU_BLEND_OP_REVERSE_SUBTRACT: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case MGPU_BLEND_OP_MIN:              return VK_BLEND_OP_MIN;
    case MGPU_BLEND_OP_MAX:              return VK_BLEND_OP_MAX;
    default: ATOM_PANIC("unhandled blend op: {}", (int)blend_op);
  }
}

inline VkVertexInputRate MGPUVertexInputRateToVkVertexInputRate(MGPUVertexInputRate vertex_input_rate) {
  switch(vertex_input_rate) {
    case MGPU_VERTEX_INPUT_RATE_VERTEX:   return VK_VERTEX_INPUT_RATE_VERTEX;
    case MGPU_VERTEX_INPUT_RATE_INSTANCE: return VK_VERTEX_INPUT_RATE_INSTANCE;
    default: ATOM_PANIC("unhandled vertex input rate: {}", (int)vertex_input_rate);
  }
}

inline VkFormat MGPUVertexFormatToVkFormat(MGPUVertexFormat vertex_format) {
  switch(vertex_format) {
    case MGPU_VERTEX_FORMAT_STUB_XYZW32323232: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case MGPU_VERTEX_FORMAT_STUB_XYZ323232: return VK_FORMAT_R32G32B32_SFLOAT;
    case MGPU_VERTEX_FORMAT_STUB_XY3232: return VK_FORMAT_R32G32_SFLOAT;
    default: ATOM_PANIC("unhandled vertex format: {}", (int)vertex_format);
  }
}

inline VkIndexType MGPUIndexFormatToVkIndexType(MGPUIndexFormat index_format) {
  switch(index_format) {
    case MGPU_INDEX_FORMAT_U16: return VK_INDEX_TYPE_UINT16;
    case MGPU_INDEX_FORMAT_U32: return VK_INDEX_TYPE_UINT32;
    default: ATOM_PANIC("unhandled index format: {}", (int)index_format);
  }
}

inline VkCompareOp MGPUCompareOpToVkCompareOp(MGPUCompareOp compare_op) {
  switch(compare_op) {
    case MGPU_COMPARE_OP_NEVER:            return VK_COMPARE_OP_NEVER;
    case MGPU_COMPARE_OP_LESS:             return VK_COMPARE_OP_LESS;
    case MGPU_COMPARE_OP_EQUAL:            return VK_COMPARE_OP_EQUAL;
    case MGPU_COMPARE_OP_LESS_OR_EQUAL:    return VK_COMPARE_OP_LESS_OR_EQUAL;
    case MGPU_COMPARE_OP_GREATER:          return VK_COMPARE_OP_GREATER;
    case MGPU_COMPARE_OP_NOT_EQUAL:        return VK_COMPARE_OP_NOT_EQUAL;
    case MGPU_COMPARE_OP_GREATER_OR_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case MGPU_COMPARE_OP_ALWAYS:           return VK_COMPARE_OP_ALWAYS;
    default: ATOM_PANIC("unhandled compare op: {}", (int)compare_op);
  }
}

inline VkStencilOp MGPUStencilOpToVkStencilOp(MGPUStencilOp stencil_op) {
  switch(stencil_op) {
    case MGPU_STENCIL_OP_KEEP: return VK_STENCIL_OP_KEEP;
    case MGPU_STENCIL_OP_ZERO: return VK_STENCIL_OP_ZERO;
    case MGPU_STENCIL_OP_REPLACE: return VK_STENCIL_OP_REPLACE;
    case MGPU_STENCIL_OP_INCREMENT_AND_CLAMP: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case MGPU_STENCIL_OP_DECREMENT_AND_CLAMP: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case MGPU_STENCIL_OP_INVERT: return VK_STENCIL_OP_INVERT;
    case MGPU_STENCIL_OP_INCREMENT_AND_WRAP: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case MGPU_STENCIL_OP_DECREMENT_AND_WRAP: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    default: ATOM_PANIC("unhandled stencil op: {}", (int)stencil_op);
  }
}

inline VkAttachmentLoadOp MGPULoadOpToVkAttachmentLoadOp(MGPULoadOp load_op) {
  switch(load_op) {
    case MGPU_LOAD_OP_CLEAR:     return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case MGPU_LOAD_OP_LOAD:      return VK_ATTACHMENT_LOAD_OP_LOAD;
    case MGPU_LOAD_OP_DONT_CARE: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    default: ATOM_PANIC("unhandled load op: {}", (int)load_op);
  }
}

inline VkAttachmentStoreOp MGPUStoreOpToVkAttachmentStoreOp(MGPUStoreOp store_op) {
  switch(store_op) {
    case MGPU_STORE_OP_STORE:     return VK_ATTACHMENT_STORE_OP_STORE;
    case MGPU_STORE_OP_DONT_CARE: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    default: ATOM_PANIC("unhandled store op: {}", (int)store_op);
  }
}

inline VkColorSpaceKHR MGPUColorSpaceToVkColorSpace(MGPUColorSpace color_space) {
  switch(color_space) {
    case MGPU_COLOR_SPACE_SRGB_NONLINEAR: return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    default: ATOM_PANIC("unhandled color space: {}", (int)color_space);
  }
}

inline VkPresentModeKHR MGPUPresentModeToVkPresentMode(MGPUPresentMode present_mode) {
  switch(present_mode) {
    case MGPU_PRESENT_MODE_IMMEDIATE:    return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case MGPU_PRESENT_MODE_MAILBOX:      return VK_PRESENT_MODE_MAILBOX_KHR;
    case MGPU_PRESENT_MODE_FIFO:         return VK_PRESENT_MODE_FIFO_KHR;
    case MGPU_PRESENT_MODE_FIFO_RELAXED: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    default: ATOM_PANIC("unhandled present mode: {}", (int)present_mode);
  }
}

}  // namespace mgpu::vulkan
